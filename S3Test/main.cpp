
#include <aws/core/Aws.h>
#include <aws/s3/S3Client.h>
#include <aws/s3/model/GetObjectRequest.h>
#include <aws/s3/model/HeadObjectRequest.h>
#include <deque>
#include <fstream>

/**
 * Get an object from an Amazon S3 bucket.
 */
int main(int argc, char** argv)
{
    Aws::SDKOptions options;
    Aws::InitAPI(options);
    const Aws::String bucket_name = "dev-full-ontime";
    const Aws::String object_name = "x0002"; // For demo, set to a text file

    // Set up the request
    Aws::S3::S3Client s3_client;

    Aws::S3::Model::HeadObjectRequest head_object_request;
    head_object_request.SetBucket(bucket_name);
    head_object_request.SetKey(object_name);
    auto outcome = s3_client.HeadObject(head_object_request);
    size_t file_size = outcome.GetResult().GetContentLength();

    // Get the object
    std::ofstream file("test.file", std::ios::binary | std::ios::trunc);
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
                object_request.SetBucket(bucket_name);
                object_request.SetKey(object_name);
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
