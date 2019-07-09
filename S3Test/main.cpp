
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
    Aws::SDKOptions options;
    Aws::InitAPI(options);
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
    Aws::ShutdownAPI(options);
}

void UploadFile(const std::string& bucket, const fs::path& file_name, const fs::path& source_file)
{
    Aws::SDKOptions options;
    Aws::InitAPI(options);
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
    std::cout << "File uploaded in " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << "ms."
              << std::endl
              << "File size: " << file_size << std::endl
              << "Speed: "
              << double(file_size) / std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() * 1000 / 1024 / 1024
              << "MiB/s" << std::endl;
    Aws::ShutdownAPI(options);
}
int main(int argc, char** argv)
{
    std::string bucket_name = "dev-full-ontime";
    std::string object_name = "ein/zwei/drei/ontime0002.csv";
    std::string target_file = "/home/ernest/Downloads/tmp-ontime/x0002";
    UploadFile(bucket_name, object_name, target_file);
    DownloadFile(bucket_name, object_name, target_file + ".dwld");
}
