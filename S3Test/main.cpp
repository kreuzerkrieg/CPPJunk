
#include <aws/core/Aws.h>
#include <aws/core/utils/stream/PreallocatedStreamBuf.h>
#include <aws/s3/S3Client.h>
#include <aws/s3/model/CompleteMultipartUploadRequest.h>
#include <aws/s3/model/CompletedMultipartUpload.h>
#include <aws/s3/model/CopyObjectRequest.h>
#include <aws/s3/model/CreateMultipartUploadRequest.h>
#include <aws/s3/model/DeleteObjectRequest.h>
#include <aws/s3/model/DeleteObjectsRequest.h>
#include <aws/s3/model/GetObjectRequest.h>
#include <aws/s3/model/HeadObjectRequest.h>
#include <aws/s3/model/ListObjectsV2Request.h>
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

class S3LazyListRetriever
{
public:
    S3LazyListRetriever(const std::string& bucket, const std::string& folder) : bucket_name(bucket), object_name(folder) {}
    auto next(size_t max_files)
    {
        Aws::S3::Model::ListObjectsV2Request list_object_request;
        list_object_request.SetBucket(bucket_name.c_str());
        list_object_request.SetPrefix(object_name.c_str());
        list_object_request.SetMaxKeys(max_files);
        if(!next_token.empty())
        {
            list_object_request.SetContinuationToken(next_token);
        }
        return DeferredProxy(*this, s3_client.ListObjectsV2Callable(list_object_request));
    }

private:
    Aws::S3::S3Client s3_client;
    Aws::S3::Model::ListObjectsV2OutcomeCallable lazy_result;
    Aws::String next_token;
    fs::path fq_name;
    std::string bucket_name;
    std::string object_name;

    class DeferredProxy
    {
    public:
        explicit DeferredProxy(S3LazyListRetriever& retriever_arg, Aws::S3::Model::ListObjectsV2OutcomeCallable&& waitable_arg)
            : retriever(retriever_arg), waitable(std::move(waitable_arg))
        {
        }

        auto get()
        {
            auto result = waitable.get();
            if(result.IsSuccess())
            {
                auto object = result.GetResultWithOwnership();
                if(object.GetIsTruncated())
                {
                    retriever.next_token = object.GetNextContinuationToken();
                }
                return std::move(object.GetContents());
            }
            else
            {
                auto error = result.GetError();
                throw std::runtime_error((Aws::String("ERROR: ") + error.GetExceptionName() + ": " + error.GetMessage()).c_str());
            }
        }

    private:
        S3LazyListRetriever& retriever;
        Aws::S3::Model::ListObjectsV2OutcomeCallable waitable;
    };
};

class S3File
{
public:
    S3File() {}
    explicit S3File(const std::string& fully_qualified_name) : S3File(fs::path(fully_qualified_name)) {}
    explicit S3File(const fs::path& fully_qualified_name) : fq_name(fully_qualified_name)
    {
        auto it = fq_name.begin();
        if(*(it) != "s3:")
        {
            throw std::runtime_error(fully_qualified_name.string() + " is not S3 fully qualified name.");
        }

        if(*(--fq_name.end()) == ".")
        {
            fq_name = fq_name.parent_path();
            it = fq_name.begin();
        }
        bucket_name = (*(++it)).string();
        ++it;

        fs::path object_path;
        for(; it != fq_name.end(); ++it)
        {
            object_path /= *it;
        }
        object_name = object_path.string();

        if(!fqn2parts(fq_name, bucket_name, object_name))
        {
            throw std::runtime_error(fully_qualified_name.string() + " is not S3 fully qualified name.");
        }
        Aws::S3::Model::ListObjectsV2Request list_object_request;
        list_object_request.SetBucket(bucket_name.c_str());
        list_object_request.SetPrefix(object_path.c_str());
        list_object_request.SetDelimiter("/");
        list_object_request.SetMaxKeys(1);
        lazy_result = s3_client.ListObjectsV2Callable(list_object_request);
    }
    bool exists()
    {
        validate();
        return is_exist;
    }
    bool isFile()
    {
        validate();
        return is_exist && is_file;
    }
    size_t size()
    {
        validate();
        return file_size;
    }
    std::chrono::system_clock::time_point lastModifiedDate()
    {
        validate();
        return last_modification_date;
    }
    void lastModifiedDate(const std::chrono::system_clock::time_point& new_last_modified) { validate(); }

    fs::path getFQName() { return fq_name; }
    S3File rename(const std::string& new_name)
    {
        validate();

        std::string target_bucket;
        std::string target_key;
        if(!fqn2parts(new_name, target_bucket, target_key))
        {
            throw std::runtime_error(new_name + " is not S3 fully qualified name.");
        }
        if(isFile())
        {
            auto source = (fs::path(bucket_name) / fs::path(object_name));
            Aws::S3::Model::CopyObjectRequest request;
            request.WithBucket(target_bucket.c_str()).WithKey(target_key.c_str()).WithCopySource(source.c_str());

            auto response = s3_client.CopyObject(request);
            if(response.IsSuccess())
            {
                return S3File(new_name);
            }
            else
            {
                // TODO EWZ: log error
                return S3File();
            }
        }
        else
        {

        }
        remove();
    }
    S3File rename(const fs::path& new_name)
    {
        validate();
        return rename(new_name.string());
    }
    bool remove()
    {
        validate();
        if(is_file)
        {
            Aws::S3::Model::DeleteObjectRequest request;
            request.WithBucket(bucket_name.c_str()).WithKey(object_name.c_str());
            auto response = s3_client.DeleteObject(request);
            if(response.IsSuccess())
            {
                return true;
            }
            else
            {
                // TODO EWZ: log error
                return false;
            }
        }
        else
        {
            Aws::S3::Model::DeleteObjectsRequest request;
            Aws::S3::Model::Delete delete_object;
            S3LazyListRetriever retriever(bucket_name, object_name);
            auto list = retriever.next(16).get();

            while(!list.empty())
            {
                Aws::Vector<Aws::S3::Model::ObjectIdentifier> objects;
                std::transform(list.begin(), list.end(), std::back_inserter(objects), [](const auto& object) {
                    Aws::S3::Model::ObjectIdentifier retVal;
                    retVal.SetKey(object.GetKey());
                    return retVal;
                });
                Aws::S3::Model::Delete delete_list;
                delete_list.SetObjects(objects);
                request.SetBucket(bucket_name.c_str());
                request.SetDelete(delete_list);
                auto result = s3_client.DeleteObjects(request);
                if(result.IsSuccess())
                {
                    list = retriever.next(16).get();
                }
                else
                {
                    auto error = result.GetError();
                    throw std::runtime_error((Aws::String("ERROR: ") + error.GetExceptionName() + ": " + error.GetMessage()).c_str());
                }
            }
        }
        setNotFound();
    }
    bool remove(bool /*recursive*/) { remove(); }

    void print()
    {
        validate();
        std::cout << "Name: " << getFQName() << ", Size: " << size() << ", Is file: " << isFile() << ", exists: " << exists()
                  << ", last mod time: " << lastModifiedDate().time_since_epoch().count() << std::endl;
    }

private:
    static bool fqn2parts(const fs::path& fqn, std::string& bucket_name, std::string& object_name)
    {
        auto it = fqn.begin();
        if(*(it) != "s3:")
        {
            return false;
        }
        bucket_name = (*(++it)).string();
        ++it;

        fs::path object_path;
        for(; it != fqn.end(); ++it)
        {
            object_path /= *it;
        }
        object_name = object_path.string();
        return true;
    }
    void validate()
    {
        if(__glibc_unlikely(lazy_result.valid()))
        {
            auto outcome = lazy_result.get();
            if(outcome.IsSuccess())
            {
                is_exist = true;
                auto result = outcome.GetResultWithOwnership();
                if(!result.GetContents().empty() && result.GetPrefix() == result.GetContents()[0].GetKey())
                {
                    is_file = true;
                    file_size = result.GetContents()[0].GetSize();
                    last_modification_date = result.GetContents()[0].GetLastModified().UnderlyingTimestamp();
                }
                else if(!result.GetCommonPrefixes().empty())
                {
                    is_file = false;
                    file_size = 0;
                }
                else
                {
                    setNotFound();
                }
            }
            else
            {
                setNotFound();
            }
        }
    }
    void setNotFound()
    {
        is_exist = false;
        is_file = false;
        file_size = 0;
        last_modification_date = std::chrono::system_clock::time_point::min();
    }
    Aws::S3::S3Client s3_client;
    Aws::S3::Model::ListObjectsV2OutcomeCallable lazy_result;
    fs::path fq_name;
    std::string bucket_name;
    std::string object_name;
    std::chrono::system_clock::time_point last_modification_date = std::chrono::system_clock::time_point::min();
    size_t file_size = 0;
    bool is_file = false;
    bool is_exist = false;
};

int main(int argc, char** argv)
{
    Aws::SDKOptions options;
    Aws::InitAPI(options);

    fs::path p("s3://dev-shadow/data/default/S3/ontime_replicated/tmp_fetch_198802_0_0_0/AirTime.bin");
    S3File file1(p);
    file1.print();
    //    fs::path new_name("s3://dev-mini-test/Renamed.file");
    //
    //    S3File file3 = file1.rename(new_name);
    //    file1.print();
    //    file3.print();
    //    file3.remove();
    //    file3.print();
    //    S3File file4(new_name);
    //    file4.print();

    S3LazyListRetriever retriever("dev-shadow", "data/default/S3/ontime_replicated/tmp_fetch_198802_0_0_0");
    auto files = retriever.next(15);
    auto results = files.get();

    fs::path p1("s3://dev-shadow/data/default/S3");
    S3File file2(p1);
    file2.print();
    file2.remove();
    file2.print();

    fs::path n;

    auto it = p.begin();
    ++it;
    for(; it != p.end(); ++it)
    {
        n /= *it;
    }
    auto a = fs::absolute(p);
    auto b = p.stem();

    std::string bucket_name = "dev-shadow";
    std::string object_name = "./ein/zwei/drei/ontime0002.csv";
    std::string target_file = "/home/ernest/Downloads/tmp-ontime/x0002";
    UploadFile(bucket_name, object_name, target_file);
    DownloadFile(bucket_name, object_name, target_file + ".dwld");
    Aws::ShutdownAPI(options);
}
