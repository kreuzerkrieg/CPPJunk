#pragma once
#include <aws/s3/S3Client.h>
#include <aws/s3/model/ListObjectsV2Request.h>
#include <filesystem>

namespace fs = std::filesystem;
class S3File;

class S3LazyListRetriever
{
public:
    class DeferredProxy
    {
    public:
        DeferredProxy(S3LazyListRetriever & retriever_arg, Aws::S3::Model::ListObjectsV2OutcomeCallable && waitable_arg);
        std::vector<S3File> get();

    private:
        S3LazyListRetriever & retriever;
        Aws::S3::Model::ListObjectsV2OutcomeCallable waitable;
    };

    S3LazyListRetriever(std::string bucket, std::string folder, bool is_recursive_);
    S3LazyListRetriever(const S3LazyListRetriever & rhs)=default;
    S3LazyListRetriever & operator=(const S3LazyListRetriever & rhs)=default;
    DeferredProxy next(size_t max_files);

private:
    Aws::String next_token;
    fs::path fq_name;
    std::string bucket_name;
    std::string object_name;
    bool have_more = true;
    bool is_recursive = false;
};