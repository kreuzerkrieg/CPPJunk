#pragma once
#include <aws/s3/S3Client.h>
#include <aws/s3/model/ListObjectsV2Request.h>
#include <experimental/filesystem>

namespace fs = std::experimental::filesystem;

class S3LazyListRetriever
{
public:
    class DeferredProxy
    {
    public:
        explicit DeferredProxy(S3LazyListRetriever& retriever_arg, Aws::S3::Model::ListObjectsV2OutcomeCallable&& waitable_arg);

        Aws::Vector<Aws::S3::Model::Object> get();

    private:
        S3LazyListRetriever& retriever;
        Aws::S3::Model::ListObjectsV2OutcomeCallable waitable;
    };

    S3LazyListRetriever(const std::string& bucket, const std::string& folder);
    DeferredProxy next(size_t max_files);

private:
    Aws::S3::S3Client s3_client;
    Aws::S3::Model::ListObjectsV2OutcomeCallable lazy_result;
    Aws::String next_token;
    fs::path fq_name;
    std::string bucket_name;
    std::string object_name;
    bool have_more = true;
};