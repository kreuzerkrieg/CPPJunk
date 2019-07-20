
#include "DirectoryIterator.h"
#include "S3File.h"
#include "S3LazyListRetriever.h"
#include <aws/core/Aws.h>
#include <aws/core/utils/stream/PreallocatedStreamBuf.h>
#include <aws/s3/S3Client.h>
#include <aws/s3/model/CompleteMultipartUploadRequest.h>
#include <aws/s3/model/CompletedMultipartUpload.h>
#include <aws/s3/model/CreateMultipartUploadRequest.h>
#include <aws/s3/model/GetObjectRequest.h>
#include <aws/s3/model/HeadObjectRequest.h>
#include <aws/s3/model/MultipartUpload.h>
#include <aws/s3/model/PutObjectRequest.h>
#include <aws/s3/model/UploadPartRequest.h>
#include <deque>
#include <experimental/filesystem>
#include <fstream>

namespace fs = std::experimental::filesystem;

void DownloadFile(const std::string& bucket, const fs::path& file_name, const fs::path& target_file)
{
    Aws::S3::S3Client s3_client;

    Aws::S3::Model::HeadObjectRequest head_object_request;
    head_object_request.SetBucket(bucket.c_str());
    head_object_request.SetKey(file_name.c_str());
    auto outcome = s3_client.HeadObject(head_object_request);
    size_t file_size = outcome.GetResult().GetContentLength();

    // Get the object
    std::ofstream file(target_file, std::ios::binary | std::ios::trunc);
    std::deque<std::future<Aws::S3::Model::GetObjectResult>> futures;
    auto begin = std::chrono::high_resolution_clock::now();

    std::promise<Aws::S3::Model::GetObjectResult> promise;
    auto future = promise.get_future();
    constexpr size_t part_size = 20 * 1024 * 1024;
    constexpr size_t threads_num = 64;
    size_t part_count = 0;
    while((part_count * part_size) <= file_size)
    {
        for(auto cycle = 0; cycle < threads_num; ++cycle)
        {
            if((part_count * part_size) >= file_size)
                break;
            futures.emplace_back(std::async(std::launch::async, [&, part{part_count++}]() {
                Aws::S3::S3Client client;
                Aws::S3::Model::GetObjectRequest object_request;
                object_request.SetBucket(bucket.c_str());
                object_request.SetKey(file_name.c_str());
                auto end_range = std::min((part + 1) * part_size - 1, file_size);
                std::string range = std::string("bytes=") + std::to_string(part * part_size) + "-" + std::to_string(end_range);
                std::cout << "Downloading part " << part << " range: from " << part * part_size << " to " << end_range << std::endl;
                object_request.SetRange(range.c_str());
                auto get_object_outcome = s3_client.GetObject(object_request);
                if(get_object_outcome.IsSuccess())
                {
                    return get_object_outcome.GetResultWithOwnership();
                }
                else
                {
                    auto error = get_object_outcome.GetError();
                    throw std::runtime_error((Aws::String("ERROR: ") + error.GetExceptionName() + ": " + error.GetMessage()).c_str());
                }
            }));
        }
        while(!futures.empty())
        {
            file << futures.front().get().GetBody().rdbuf();
            futures.pop_front();
        }
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "File downloaded and saved in " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << "ms."
              << std::endl
              << "File size: " << file_size << std::endl
              << "Speed: "
              << double(file_size) / std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() * 1000 / 1024 / 1024
              << "MiB/s" << std::endl;
}

void UploadFile(const std::string& bucket, const fs::path& file_name, const fs::path& source_file)
{
    Aws::S3::S3Client s3_client;

    Aws::Vector<Aws::S3::Model::CompletedPart> parts;
    std::ifstream file(source_file, std::ios::binary);
    std::deque<std::future<Aws::S3::Model::CompletedPart>> futures;
    auto begin = std::chrono::high_resolution_clock::now();

    std::promise<void> promise;
    auto future = promise.get_future();
    constexpr size_t part_size = 20 * 1024 * 1024;
    constexpr size_t threads_num = 64;
    size_t part_count = 0;

    file.seekg(0, std::ios_base::end);
    const size_t file_size = static_cast<size_t>(file.tellg());
    file.seekg(0, std::ios_base::beg);

    Aws::S3::Model::CreateMultipartUploadRequest createMultipartRequest;
    createMultipartRequest.WithBucket(bucket.c_str());
    createMultipartRequest.WithKey(file_name.c_str());
    auto createMultipartResponse = s3_client.CreateMultipartUpload(createMultipartRequest);
    Aws::String upload_id;
    if(createMultipartResponse.IsSuccess())
    {
        upload_id = createMultipartResponse.GetResult().GetUploadId();
    }
    else
    {
        auto error = createMultipartResponse.GetError();
        throw std::runtime_error((Aws::String("ERROR: ") + error.GetExceptionName() + ": " + error.GetMessage()).c_str());
    }
    while((part_count * part_size) <= file_size)
    {
        for(auto cycle = 0; cycle < threads_num; ++cycle)
        {
            if((part_count * part_size) >= file_size)
                break;
            futures.emplace_back(std::async(std::launch::async, [&, part{part_count++}]() {
                Aws::S3::Model::UploadPartRequest object_request;
                object_request.SetBucket(bucket.c_str());
                object_request.SetKey(file_name.c_str());
                object_request.SetPartNumber(part + 1);
                object_request.SetUploadId(upload_id);

                auto start_pos = part * part_size;
                auto end_pos = std::min((part + 1) * part_size, file_size);
                object_request.SetContentLength(end_pos - start_pos);

                std::vector<uint8_t> buff(object_request.GetContentLength());
                file.seekg(start_pos);
                file.read(reinterpret_cast<char*>(buff.data()), buff.size());
                Aws::Utils::Stream::PreallocatedStreamBuf stream_buff(buff.data(), buff.size());

                object_request.SetBody(Aws::MakeShared<Aws::IOStream>("Uploader", &stream_buff));

                auto response = s3_client.UploadPart(object_request);
                if(response.IsSuccess())
                {
                    Aws::S3::Model::CompletedPart part;
                    part.SetPartNumber(object_request.GetPartNumber());
                    part.SetETag(response.GetResult().GetETag());
                    return std::move(part);
                }
                else
                {
                    auto error = response.GetError();
                    throw std::runtime_error((Aws::String("ERROR: ") + error.GetExceptionName() + ": " + error.GetMessage()).c_str());
                }
            }));
        }
        while(!futures.empty())
        {
            parts.emplace_back(futures.front().get());
            futures.pop_front();
        }
    }
    std::sort(parts.begin(), parts.end(), [](const auto& lhs, const auto& rhs) { return lhs.GetPartNumber() < rhs.GetPartNumber(); });
    Aws::S3::Model::CompletedMultipartUpload completed;
    completed.SetParts(std::move(parts));

    Aws::S3::Model::CompleteMultipartUploadRequest complete_request;
    complete_request.SetBucket(bucket.c_str());
    complete_request.SetKey(file_name.c_str());
    complete_request.SetUploadId(upload_id);
    complete_request.SetMultipartUpload(completed);
    auto result = s3_client.CompleteMultipartUpload(complete_request);
    if(!result.IsSuccess())
    {
        auto error = result.GetError();
        throw std::runtime_error((Aws::String("ERROR: ") + error.GetExceptionName() + ": " + error.GetMessage()).c_str());
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "File uploaded in " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << "ms." << std::endl
              << "File size: " << file_size << std::endl
              << "Speed: "
              << double(file_size) / std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() * 1000 / 1024 / 1024
              << "MiB/s" << std::endl;
}

int main(int argc, char** argv)
{
    Aws::SDKOptions options;
    Aws::InitAPI(options);
    /*    fs::path path("s3:/dev-shadow/./data/default//S3/ontime_replicated/tmp_fetch_198711_0_0_0/.");
        fs::path clean;
        for(const auto& part : path)
        {
            if(part == ".")
            {
                continue;
            }
            if(part == "..")
            {
                clean = clean.parent_path();
                continue;
            }
            clean /= part;
        }
        clean /= "";
        std::cout << path.string() << std::endl;
        std::cout << clean.string() << std::endl;
        std::cout << path.generic_string() << std::endl;
        std::cout << path.parent_path().string() << std::endl;
        std::cout << path.stem().string() << std::endl;
        std::cout << path.native() << std::endl;
        std::cout << path.root_path().native() << std::endl;
        std::cout << path.root_directory().native() << std::endl;
        std::cout << path.root_name().native() << std::endl;
        std::cout << fs::absolute(path).string() << std::endl;
        //    std::cout << fs::canonical(path).string() << std::endl;
        std::cout << fs::system_complete(path).string() << std::endl;

        {
            S3File f1(fs::path("s3://dev-shadow/ein/zwei/"));
            std::cout << f1.getFQName() << " is a " << (f1.isFile() ? " file." : " folder.") << std::endl;
        }
        {
            S3File f1(fs::path("s3://dev-shadow/ein/zwei"));
            std::cout << f1.getFQName() << " is a " << (f1.isFile() ? " file." : " folder.") << std::endl;
        }

        {
            S3File f1(fs::path("s3://dev-shadow/ein/zwei.bin"));
            std::cout << f1.getFQName() << " is a " << (f1.isFile() ? " file." : " folder.") << std::endl;
        }

        {
            // Test folder copy to another folder
            auto source = S3File(std::string("s3://dev-shadow/one/"));
            source.copy(std::string("s3://dev-shadow/ein/zwei/"));
        }*/

    // fs::path test_path("s3://dev-shadow/data/default/S3/ontime_replicated/tmp_fetch_198711_0_0_0/columns.txt");

    //    S3File source_file(std::string("s3:/dev-shadow/data/default/S3/ontime_replicated/tmp_fetch_198711_0_0_0/columns.txt"));
    //    std::cout << source_file.exists() << std::endl;
    //    source_file.print();
    //    // Test copy
    //    auto copied = source_file.copy("s3://dev-shadow/ein/zwei/test_file.txt");
    //    // Test copy to another folder
    //    auto ccopied = copied.copy("s3://dev-shadow/one/two/test_file.txt");
    //
    //    // Test folder copy to another folder
    //    auto source = S3File(std::string("s3://dev-shadow/one/"));
    //    source.copy(std::string("s3://dev-shadow/ein/zwei/"));
    //
    //    // Test file rename
    //    ccopied.rename(std::string("s3://dev-shadow/one/two/renamed_file.txt"));
    //
    //    // Test folder rename
    //    S3File source_folder(fs::path("s3://dev-shadow/ein/zwei/"));
    //    auto new_folder = source_folder.rename(std::string("s3://dev-shadow/ein/dos/"));
    //
    //    // Cleanup and test deletion
    //    new_folder.remove();

    //    fs::path p("s3://dev-shadow/data/default/S3/ontime_replicated/tmp_fetch_198802_0_0_0/AirTime.bin");
    //    S3File file1(p);
    //    file1.print();
    //    fs::path new_name("s3://dev-mini-test/Renamed.file");
    //
    //    S3File file3 = file1.rename(new_name);
    //    file1.print();
    //    file3.print();
    //    file3.remove();
    //    file3.print();
    //    S3File file4(new_name);
    //    file4.print();

    //    S3LazyListRetriever retriever("dev-shadow", "data/default/S3/ontime_replicated/tmp_fetch_198802_0_0_0");
    //    auto files = retriever.next(15);
    //    auto results = files.get();
    //
    //    fs::path p1("s3://dev-shadow/data/default/S3");
    //    S3File file2(p1);
    //    file2.print();
    //    file2.remove();
    //    file2.print();
    //
    //    fs::path n;
    //
    //    auto it = p.begin();
    //    ++it;
    //    for(; it != p.end(); ++it)
    //    {
    //        n /= *it;
    //    }
    //    auto a = fs::absolute(p);
    //    auto b = p.stem();
    //
    std::string bucket_name = "dev-shadow";
    std::string object_name = "./ein/zwei/drei/ontime0002.csv";
    //    std::string target_file = "/home/ernest/Downloads/tmp-ontime/x0002";
    //    UploadFile(bucket_name, object_name, target_file);
    // DownloadFile(bucket_name, object_name, target_file + ".dwld");
    for(auto& item : DirectoryIterator("s3://dev-shadow/dev-shadow/data/default/S3/ontime_replicated/"))
    {
        std::cout << item.getFQName() << std::endl;
    }
    Aws::ShutdownAPI(options);
}
