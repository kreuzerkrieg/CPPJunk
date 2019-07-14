#include "S3LazyListRetriever.h"

S3LazyListRetriever::S3LazyListRetriever(const std::string& bucket, const std::string& folder) : bucket_name(bucket), object_name(folder) {}
S3LazyListRetriever::DeferredProxy S3LazyListRetriever::next(size_t max_files)
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
S3LazyListRetriever::DeferredProxy::DeferredProxy(S3LazyListRetriever& retriever_arg,
                                                  Aws::S3::Model::ListObjectsV2OutcomeCallable&& waitable_arg)
    : retriever(retriever_arg), waitable(std::move(waitable_arg))
{
}
Aws::Vector<Aws::S3::Model::Object> S3LazyListRetriever::DeferredProxy::get()
{
    if(!retriever.have_more)
    {
        return {};
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
        return std::move(object.GetContents());
    }
    else
    {
        auto error = result.GetError();
        throw std::runtime_error((Aws::String("ERROR: ") + error.GetExceptionName() + ": " + error.GetMessage()).c_str());
    }
}
