#include "Uploader.h"
#include "Fs.h"
#include <aws/core/utils/threading/Executor.h>
#include <aws/s3/S3Client.h>
#include <aws/s3/model/CompleteMultipartUploadRequest.h>
#include <aws/s3/model/CompletedMultipartUpload.h>
#include <aws/s3/model/CompletedPart.h>
#include <aws/s3/model/CreateMultipartUploadRequest.h>
#include <aws/s3/model/UploadPartRequest.h>
#include <deque>
#include <fstream>

using namespace std::chrono;

Uploader::Uploader() {}

class OwningStreamBuf : public std::streambuf
{
public:
    explicit OwningStreamBuf(Aws::Vector<char>&& buffer) : m_underlyingBuffer(std::move(buffer))
    {
        char* end = m_underlyingBuffer.data() + m_underlyingBuffer.size();
        char* begin = m_underlyingBuffer.data();
        setp(begin, end);
        setg(begin, begin, end);
    }

    OwningStreamBuf(const OwningStreamBuf&) = delete;
    OwningStreamBuf& operator=(const OwningStreamBuf&) = delete;

    OwningStreamBuf(OwningStreamBuf&& toMove) = delete;
    OwningStreamBuf& operator=(OwningStreamBuf&&) = delete;

    char* GetBuffer() { return m_underlyingBuffer.data(); }

protected:
    pos_type seekoff(off_type off, std::ios_base::seekdir dir,
                     std::ios_base::openmode which = std::ios_base::in | std::ios_base::out) override
    {
        if(dir == std::ios_base::beg)
        {
            return seekpos(off, which);
        }
        else if(dir == std::ios_base::end)
        {
            return seekpos(m_underlyingBuffer.size() - off, which);
        }
        else if(dir == std::ios_base::cur)
        {
            if(which == std::ios_base::in)
            {
                return seekpos((gptr() - m_underlyingBuffer.data()) + off, which);
            }
            else
            {
                return seekpos((pptr() - m_underlyingBuffer.data()) + off, which);
            }
        }

        return off_type(-1);
    }
    pos_type seekpos(pos_type pos, std::ios_base::openmode which = std::ios_base::in | std::ios_base::out) override
    {
        assert(static_cast<size_t>(pos) <= m_underlyingBuffer.size());
        if(static_cast<size_t>(pos) > m_underlyingBuffer.size())
        {
            return pos_type(off_type(-1));
        }

        char* end = m_underlyingBuffer.data() + m_underlyingBuffer.size();
        char* begin = m_underlyingBuffer.data();

        if(which == std::ios_base::in)
        {
            setg(begin, begin + static_cast<size_t>(pos), end);
        }

        if(which == std::ios_base::out)
        {
            setp(begin + static_cast<size_t>(pos), end);
        }

        return pos;
    }

private:
    Aws::Vector<char> m_underlyingBuffer;
};

void Uploader::upload(const fs::path& fully_qualified_name, const fs::path& source_file)
{
    struct UploadRequest
    {
        Aws::S3::Model::UploadPartOutcomeCallable future;
        std::unique_ptr<OwningStreamBuf> buffer;
    };

    Aws::Client::ClientConfiguration config;
    config.executor = Aws::MakeShared<Aws::Utils::Threading::PooledThreadExecutor>("balh", std::thread::hardware_concurrency());
    Aws::S3::S3Client s3_client(config);

    std::deque<UploadRequest> part_completion;
    Aws::Vector<Aws::S3::Model::CompletedPart> completed_parts;

    std::string bucket;
    std::string file_name;
    S3fqn2parts(fully_qualified_name, bucket, file_name);

    Aws::S3::Model::CreateMultipartUploadRequest create_multipart_request;
    create_multipart_request.WithBucket(bucket.c_str());
    create_multipart_request.WithKey(file_name.c_str());
    auto create_multipart_response = s3_client.CreateMultipartUpload(create_multipart_request);
    Aws::String upload_id;
    if(create_multipart_response.IsSuccess())
    {
        upload_id = create_multipart_response.GetResult().GetUploadId();
    }
    else
    {
        const auto& error = create_multipart_response.GetError();
        throw std::runtime_error((Aws::String("ERROR: ") + error.GetExceptionName() + ": " + error.GetMessage()).c_str());
    }

    size_t part_count = 0;

    std::ifstream file(source_file, std::ios::binary);
    file.seekg(0, std::ios_base::end);
    const size_t file_size = static_cast<size_t>(file.tellg());
    file.seekg(0, std::ios_base::beg);

    auto begin = std::chrono::high_resolution_clock::now();

    while((part_count * part_size) <= file_size && !abort)
    {
        Aws::S3::Model::UploadPartRequest object_request;
        object_request.SetBucket(bucket.c_str());
        object_request.SetKey(file_name.c_str());
        object_request.SetPartNumber(part_count + 1);
        object_request.SetUploadId(upload_id);

        auto start_pos = part_count * part_size;
        auto end_pos = std::min((part_count + 1) * part_size, file_size);
        object_request.SetContentLength(end_pos - start_pos);

        Aws::Vector<char> buff(object_request.GetContentLength());
        file.seekg(start_pos);
        file.read(buff.data(), buff.size());

        UploadRequest ur;
        ur.buffer = std::make_unique<OwningStreamBuf>(std::move(buff));
        object_request.SetBody(Aws::MakeShared<Aws::IOStream>("Uploader", ur.buffer.get()));
        ur.future = s3_client.UploadPartCallable(object_request);

        part_completion.emplace_back(std::move(ur));
        ++part_count;
        if(part_completion.size() > std::thread::hardware_concurrency() * 2)
        {
            auto result = part_completion.front().future.get();
            if(result.IsSuccess())
            {
                Aws::S3::Model::CompletedPart part;
                part.SetPartNumber(completed_parts.size() + 1);
                part.SetETag(result.GetResult().GetETag());
                completed_parts.emplace_back(std::move(part));
            }
            else
            {
                const auto& error = result.GetError();
                throw std::runtime_error((Aws::String("ERROR: ") + error.GetExceptionName() + ": " + error.GetMessage()).c_str());
            }
            part_completion.pop_front();
        }
    }

    while(!part_completion.empty())
    {
        auto result = part_completion.front().future.get();
        if(result.IsSuccess())
        {
            Aws::S3::Model::CompletedPart part;
            part.SetPartNumber(completed_parts.size() + 1);
            part.SetETag(result.GetResult().GetETag());
            completed_parts.emplace_back(std::move(part));
        }
        else
        {
            const auto& error = result.GetError();
            throw std::runtime_error((Aws::String("ERROR: ") + error.GetExceptionName() + ": " + error.GetMessage()).c_str());
        }
        part_completion.pop_front();
    }

    Aws::S3::Model::CompletedMultipartUpload completed;
    completed.SetParts(std::move(completed_parts));

    Aws::S3::Model::CompleteMultipartUploadRequest complete_request;
    complete_request.SetBucket(bucket.c_str());
    complete_request.SetKey(file_name.c_str());
    complete_request.SetUploadId(upload_id);
    complete_request.SetMultipartUpload(completed);
    auto result = s3_client.CompleteMultipartUpload(complete_request);
    if(!result.IsSuccess())
    {
        const auto& error = result.GetError();
        throw std::runtime_error((Aws::String("ERROR: ") + error.GetExceptionName() + ": " + error.GetMessage()).c_str());
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "File uploaded in " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << "ms." << std::endl
              << "File size: " << file_size << std::endl
              << "Speed: "
              << double(file_size) / std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() * 1000 / 1024 / 1024
              << "MiB/s" << std::endl;
}
