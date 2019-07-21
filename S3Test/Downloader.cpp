#include "Downloader.h"
#include "Fs.h"
#include "S3File.h"
#include <aws/s3/S3Client.h>
#include <aws/s3/model/GetObjectRequest.h>
#include <aws/s3/model/HeadObjectRequest.h>
#include <deque>
#include <fstream>

void FileDownloader::Download(const fs::path& fully_qualified_name, const fs::path& target_file)
{
    std::lock_guard lck(download_in_progress);

    downloading_thread = std::thread([this, &fully_qualified_name]() {
        S3File s3file(fully_qualified_name);
        size_t file_size = s3file.size();

        constexpr size_t part_size = 1 * 1024 * 1024;
        constexpr size_t parts_num = 64;
        size_t part_count = 0;
        while((part_count * part_size) <= file_size || !abort)
        {
            if((part_count * part_size) >= file_size)
            {
                has_work_to_do = false;
                break;
            }

            auto part = part_count++;
            std::string bucket_name;
            std::string object_name;
            S3fqn2parts(fully_qualified_name, bucket_name, object_name);
            Aws::S3::Model::GetObjectRequest object_request;
            object_request.SetBucket(bucket_name.c_str());
            object_request.SetKey(object_name.c_str());
            auto end_range = std::min((part + 1) * part_size - 1, file_size);
            std::string range = std::string("bytes=") + std::to_string(part * part_size) + "-" + std::to_string(end_range);
            std::cout << "Downloading part " << part << " range: from " << part * part_size << " to " << end_range << std::endl;
            object_request.SetRange(range.c_str());

            {
                std::unique_lock<std::mutex> lck(download_mtx);
                cv.wait(lck, [this] { return part_completion.size() < parts_num; });
                part_completion.emplace_back(s3_client.GetObjectCallable(object_request));
            }

            cv.notify_all();
        }
    });

    std::ofstream file(target_file, std::ios::binary | std::ios::trunc);
    auto begin = std::chrono::high_resolution_clock::now();

    while(has_work_to_do || !abort)
    {
        Aws::S3::Model::GetObjectResult part;
        {
            std::unique_lock<std::mutex> lck(download_mtx);
            cv.wait(lck, [this] { return !part_completion.empty(); });

            auto result = part_completion.front().get();
            if(!result.IsSuccess())
            {
                abort = true;
                auto error = result.GetError();
                throw std::runtime_error((Aws::String("ERROR: ") + error.GetExceptionName() + ": " + error.GetMessage()).c_str());
            }
            part = result.GetResultWithOwnership();
            part_completion.pop_front();
        }
        file << part.GetBody().rdbuf();
        cv.notify_all();
    }
    auto end = std::chrono::high_resolution_clock::now();

    S3File s3file(fully_qualified_name);
    size_t file_size = s3file.size();
    std::cout << "File downloaded and saved in " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << "ms."
              << std::endl
              << "File size: " << file_size << std::endl
              << "Speed: "
              << double(file_size) / std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() * 1000 / 1024 / 1024
              << "MiB/s" << std::endl;
    downloading_thread.join();
}
