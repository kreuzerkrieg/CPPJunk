#include "S3MultipartUploader.h"
#include "Utils.h"
#include <aws/core/utils/stream/PreallocatedStreamBuf.h>
#include <aws/s3/model/AbortMultipartUploadRequest.h>
#include <aws/s3/model/CompleteMultipartUploadRequest.h>
#include <aws/s3/model/CompletedMultipartUpload.h>
#include <aws/s3/model/CreateMultipartUploadRequest.h>
#include <aws/s3/model/UploadPartRequest.h>


S3MultipartUploader::S3MultipartUploader(const std::string& bucket_arg, const std::string& file_name_arg)
    : bucket(bucket_arg),
      file_name(file_name_arg)
{
    Aws::S3::Model::CreateMultipartUploadRequest create_multipart_request;
    create_multipart_request.WithBucket(bucket_arg).WithKey(file_name_arg);

    auto create_multipart_response = getS3Client().CreateMultipartUpload(create_multipart_request);

    if(create_multipart_response.IsSuccess())
    {
        upload_id = create_multipart_response.GetResult().GetUploadId();
    }
    else
    {
        const auto& error = create_multipart_response.GetError();
        throw std::runtime_error(("Exception thrown for '" + parts2s3fqn(bucket, file_name).string() + "' - " +
                                  error.GetExceptionName() + ": " + error.GetMessage())
            );
    }
}

S3MultipartUploader::~S3MultipartUploader()
{
    if(!part_completion.empty())
    {
        std::cout << "Upload of " << parts2s3fqn(bucket, file_name) << " was not finalized. It may be a bug."
            << std::endl;
        {
            while(!part_completion.empty())
            {
                try
                {
                    auto result = part_completion.front().future.get();
                }
                catch(...)
                {
                }
                part_completion.pop_front();
            }
        }

        Aws::S3::Model::AbortMultipartUploadRequest abort_multipart_request;
        abort_multipart_request.WithBucket(bucket).WithKey(file_name).WithUploadId(upload_id);
        auto abort_multipart_response = getS3Client().AbortMultipartUpload(abort_multipart_request);
        if(!abort_multipart_response.IsSuccess())
        {
            const auto& error = abort_multipart_response.GetError();
            std::cout << "Exception thrown for '" << parts2s3fqn(bucket, file_name) << "' - " << error.GetExceptionName()
                << ": " << error.GetMessage();
        }
    }
}

void S3MultipartUploader::uploadPart(const char* buff_arg, size_t buff_size)
{
    uploadPart(std::vector<char>(buff_arg, buff_arg + buff_size));
}

void S3MultipartUploader::uploadPart(std::vector<char>&& buff_arg)
{
    Aws::S3::Model::UploadPartRequest object_request;
    object_request.SetBucket(bucket);
    object_request.SetKey(file_name);
    object_request.SetPartNumber(part_number++);
    object_request.SetUploadId(upload_id);
    object_request.SetContentLength(buff_arg.size());

    PartData pd;
    pd.part_buffer = std::move(buff_arg);
    pd.part_stream = std::make_unique<Aws::Utils::Stream::PreallocatedStreamBuf>(
        reinterpret_cast<unsigned char*>(pd.part_buffer.data()), pd.part_buffer.size());
    pd.part_num = object_request.GetPartNumber();

    object_request.SetBody(std::make_shared<Aws::IOStream>(pd.part_stream.get()));

    pd.future = getS3Client().UploadPartCallable(object_request);
    part_completion.emplace_back(std::move(pd));
}

void S3MultipartUploader::finalizeUpload()
{
    while(!part_completion.empty())
    {
        finalizePart();
    }

    Aws::S3::Model::CompletedMultipartUpload completed;
    completed.SetParts(std::move(completed_parts));

    Aws::S3::Model::CompleteMultipartUploadRequest complete_request;
    complete_request.SetBucket(bucket);
    complete_request.SetKey(file_name);
    complete_request.SetUploadId(upload_id);
    complete_request.SetMultipartUpload(std::move(completed));
    auto result = getS3Client().CompleteMultipartUpload(complete_request);
    if(!result.IsSuccess())
    {
        const auto& error = result.GetError();
        throw std::runtime_error(("Exception thrown for '" + parts2s3fqn(bucket, file_name).string() + "' - " +
                                  error.GetExceptionName() + ": " + error.GetMessage() + "\nWhen trying to complete " +
                                  std::to_string(complete_request.GetMultipartUpload().GetParts().size()) + " parts.")
            );
    }
}

size_t S3MultipartUploader::finalizePart()
{
    size_t ret_val = 0;
    if(!part_completion.empty())
    {
        ret_val = part_completion.front().part_buffer.size();
        auto result = part_completion.front().future.get();
        if(result.IsSuccess())
        {
            Aws::S3::Model::CompletedPart part;
            part.SetPartNumber(part_completion.front().part_num);
            part.SetETag(result.GetResult().GetETag());
            completed_parts.emplace_back(std::move(part));
        }
        else
        {
            const auto& error = result.GetError();
            throw std::runtime_error(("Exception thrown for '" + parts2s3fqn(bucket, file_name).string() +
                                      "' - " + error.GetExceptionName() + ": " + error.GetMessage() + "\nWhen trying to complete " +
                                      std::to_string(part_completion.size()) + " parts.")
                );
        }
        part_completion.pop_front();
    }
    return ret_val;
}