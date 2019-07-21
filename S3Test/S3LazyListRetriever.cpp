#include "S3LazyListRetriever.h"

S3LazyListRetriever::S3LazyListRetriever(const std::string& bucket, const std::string& folder, bool is_recursive_)
    : bucket_name(bucket), object_name(folder), is_recursive(is_recursive_)
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
            list_object_request.SetDelimiter("/");
        list_object_request.SetMaxKeys(max_files);
        if(!next_token.empty())
            list_object_request.SetContinuationToken(next_token);
        return DeferredProxy(*this, s3_client.ListObjectsV2Callable(list_object_request));
    }
    else
    {
        return DeferredProxy(*this, {});
    }
}
S3LazyListRetriever::S3LazyListRetriever(const S3LazyListRetriever& rhs) { *this = rhs; }
S3LazyListRetriever& S3LazyListRetriever::operator=(const S3LazyListRetriever& rhs)
{
    next_token = rhs.next_token;
    fq_name = rhs.fq_name;
    bucket_name = rhs.bucket_name;
    object_name = rhs.object_name;
    have_more = rhs.have_more;
    is_recursive = rhs.is_recursive;

    return *this;
}
S3LazyListRetriever::DeferredProxy::DeferredProxy(S3LazyListRetriever& retriever_arg,
                                                  Aws::S3::Model::ListObjectsV2OutcomeCallable&& waitable_arg)
    : retriever(retriever_arg), waitable(std::move(waitable_arg))
{
}
Aws::Vector<Aws::String> S3LazyListRetriever::DeferredProxy::get()
{
    if(!retriever.have_more)
    {
        return {};
    }

    if(!waitable.valid())
        throw std::runtime_error("Something went terribly wrong");
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
        Aws::Vector<Aws::String> ret_val;
        std::transform(object.GetContents().begin(), object.GetContents().end(), std::back_inserter(ret_val),
                       [](const auto& item) { return item.GetKey(); });

        std::transform(object.GetCommonPrefixes().begin(), object.GetCommonPrefixes().end(), std::back_inserter(ret_val),
                       [](const auto& item) { return item.GetPrefix(); });

        return ret_val;
    }
    else
    {
        auto error = result.GetError();
        throw std::runtime_error((Aws::String("ERROR: ") + error.GetExceptionName() + ": " + error.GetMessage()).c_str());
    }
}
