#include "DownloaderV2.h"
#include "Fs.h"
#include "S3File.h"
#include <aws/core/utils/threading/Executor.h>
#include <aws/s3/S3Client.h>
#include <aws/s3/model/GetObjectRequest.h>
#include <aws/s3/model/HeadObjectRequest.h>
#include <deque>
#include <fstream>

using namespace std::chrono;

void FileDownloaderV2::download(const fs::path& fully_qualified_name, const fs::path& target_file) const
{
    auto begin = std::chrono::high_resolution_clock::now();

    S3File s3file(fully_qualified_name);
    size_t file_size = s3file.size();

    std::string bucket_name;
    std::string object_name;
    S3fqn2parts(fully_qualified_name, bucket_name, object_name);

    const size_t number_of_parallel_parts = std::thread::hardware_concurrency() * 2;
    constexpr size_t part_size = 10 * 1024 * 1024;
    size_t part_count = 0;

    std::ofstream file(target_file, std::ios::binary | std::ios::trunc);

    Aws::Client::ClientConfiguration config;
    config.executor = Aws::MakeShared<Aws::Utils::Threading::PooledThreadExecutor>("blah", std::thread::hardware_concurrency());
    Aws::S3::S3Client s3_client(config);

    std::deque<Aws::S3::Model::GetObjectOutcomeCallable> part_completion;
    while((part_count * part_size) <= file_size && !abort_ops)
    {
        Aws::S3::Model::GetObjectRequest object_request;
        object_request.SetBucket(bucket_name.c_str());
        object_request.SetKey(object_name.c_str());
        auto end_range = std::min((part_count + 1) * part_size - 1, file_size);
        std::string range = std::string("bytes=") + std::to_string(part_count * part_size) + "-" + std::to_string(end_range);
        object_request.SetRange(range.c_str());
        part_completion.emplace_back(s3_client.GetObjectCallable(object_request));
        ++part_count;
        if(part_completion.size() >= number_of_parallel_parts)
        {
            popAndSave(part_completion, file);
        }
    }

    while(!part_completion.empty() && !abort_ops)
    {
        popAndSave(part_completion, file);
    }
    auto end = std::chrono::high_resolution_clock::now();

    std::cout << "File downloaded and saved in " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << "ms."
              << std::endl
              << "File size: " << file_size << std::endl
              << "Speed: "
              << double(file_size) / std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() * 1000 / 1024 / 1024
              << "MiB/s" << std::endl;
}

void FileDownloaderV2::abort() { abort_ops = true; }

void FileDownloaderV2::popAndSave(std::deque<Aws::S3::Model::GetObjectOutcomeCallable>& completion_queue, std::ofstream& file) const
{
    if(!completion_queue.empty())
    {
        auto result = completion_queue.front().get();
        if(!result.IsSuccess())
        {
            const auto& error = result.GetError();
            throw std::runtime_error((Aws::String("ERROR: ") + error.GetExceptionName() + ": " + error.GetMessage()).c_str());
        }
        auto part = result.GetResultWithOwnership();
        completion_queue.pop_front();
        file << part.GetBody().rdbuf();
    }
}
