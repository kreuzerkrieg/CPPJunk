#pragma once
#include <aws/s3/S3Client.h>
#include <deque>
#include <string>

namespace Aws::S3::Model
{
class CompletedPart;
}

namespace Aws::Utils::Stream
{
class PreallocatedStreamBuf;
}

class S3MultipartUploader
{
public:
    S3MultipartUploader(const std::string& bucket_arg, const std::string& file_name_arg);

    S3MultipartUploader(const S3MultipartUploader&) = delete;
    S3MultipartUploader(S3MultipartUploader&&) = delete;

    S3MultipartUploader& operator=(const S3MultipartUploader&) = delete;
    S3MultipartUploader& operator=(S3MultipartUploader&&) = delete;

    ~S3MultipartUploader();
    void uploadPart(const char* buff_arg, size_t buff_size);
    void uploadPart(std::vector<char>&& buff_arg);
    size_t finalizePart();
    void finalizeUpload();

private:
    struct PartData
    {
        std::vector<char> part_buffer;
        std::unique_ptr<Aws::Utils::Stream::PreallocatedStreamBuf> part_stream;
        Aws::S3::Model::UploadPartOutcomeCallable future;
        size_t part_num = 0;
    };
    Aws::String upload_id;
    Aws::String bucket;
    Aws::String file_name;
    Aws::Vector<Aws::S3::Model::CompletedPart> completed_parts;
    std::deque<PartData> part_completion;
    size_t part_number = 1;
};
