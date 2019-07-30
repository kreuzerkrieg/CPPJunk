#include "S3LazyListRetriever.h"
#include "S3File.h"
#include "Utils.h"
#include <utility>

S3LazyListRetriever::S3LazyListRetriever(std::string bucket, std::string folder, bool is_recursive_)
    : bucket_name(std::move(bucket)), object_name(std::move(folder)), is_recursive(is_recursive_)
{
}
S3LazyListRetriever::DeferredProxy S3LazyListRetriever::next(size_t max_files)
{
    if(!next_token.empty() || have_more)
    {
        Aws::S3::Model::ListObjectsV2Request list_object_request;
        list_object_request.SetBucket(bucket_name.c_str());
        list_object_request.SetPrefix(object_name.c_str());
        if(!is_recursive)
        {
            list_object_request.SetDelimiter("/");
        }
        list_object_request.SetMaxKeys(max_files);
        if(!next_token.empty())
        {
            list_object_request.SetContinuationToken(next_token);
        }
        return DeferredProxy(*this, getS3Client().ListObjectsV2Callable(list_object_request));
    }
    else
    {
        return DeferredProxy(*this, {});
    }
}
S3LazyListRetriever::DeferredProxy::DeferredProxy(S3LazyListRetriever& retriever_arg,
                                                  Aws::S3::Model::ListObjectsV2OutcomeCallable&& waitable_arg)
    : retriever(retriever_arg), waitable(std::move(waitable_arg))
{
}
std::vector<S3File> S3LazyListRetriever::DeferredProxy::get()
{
    if(!retriever.have_more)
    {
        return {};
    }

    if(!waitable.valid())
    {
        throw std::runtime_error("Something went terribly wrong");
    }

    auto result = waitable.get();
    if(result.IsSuccess())
    {
        auto object = result.GetResultWithOwnership();
        if(object.GetIsTruncated())
        {
            retriever.next_token = object.GetNextContinuationToken();
        }
        else
        {
            retriever.have_more = false;
        }
        std::vector<S3File> ret_val;
        std::transform(object.GetContents().begin(), object.GetContents().end(), std::back_inserter(ret_val),
                       [this](const auto& item) { return S3File(retriever.bucket_name, item); });

        std::transform(object.GetCommonPrefixes().begin(), object.GetCommonPrefixes().end(), std::back_inserter(ret_val),
                       [this](const auto& item) { return S3File(parts2s3fqn(retriever.bucket_name, item.GetPrefix().c_str())); });

        return ret_val;
    }
    else
    {
        const auto& error = result.GetError();
        std::string error_message = (Aws::String("ERROR: ") + error.GetExceptionName() + ": " + error.GetMessage()).c_str();
        throw std::runtime_error(error_message);
    }
}
