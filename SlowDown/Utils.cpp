#include "Utils.h"
#include <aws/core/auth/AWSCredentials.h>
#include <aws/core/client/ClientConfiguration.h>
#include <aws/core/client/DefaultRetryStrategy.h>
#include <aws/s3/S3Client.h>

Aws::S3::S3Client& getS3Client()
{
    static Aws::Client::ClientConfiguration clientConfig;
    //    clientConfig.caFile = "/etc/ssl/certs/ca-certificates.crt";
    clientConfig.maxConnections = 64;
    clientConfig.connectTimeoutMs = 50;
    clientConfig.requestTimeoutMs = 50;
    clientConfig.region = "us-east-1";
    clientConfig.retryStrategy = std::make_shared<Aws::Client::DefaultRetryStrategy>(15, 10);

    static Aws::S3::S3Client client(clientConfig);
    return client;
}

fs::path s3canonicalize(const fs::path& path)
{
    // the original "s3://" will be affected by canonicalization to the form of "s3:/", so, just cut it and add later
    auto canonical = path.lexically_normal().string().substr(4);
    return "s3://" + canonical;
}

fs::path parts2s3fqn(const std::string& bucket_name, const std::string& object_name)
{
    std::string tmp = "s3://";
    tmp += bucket_name;
    tmp += fs::path::preferred_separator;
    tmp += object_name;
    return s3canonicalize(tmp);
}

bool s3fqn2parts(const fs::path& fqn, std::string& bucket_name, std::string& object_name)
{
    if(!isS3fqn(fqn))
    {
        return false;
    }

    auto name = s3canonicalize(fqn);
    auto it = name.begin();

    bucket_name = (*(++it)).string();
    ++it;

    fs::path object_path;
    for(; it != name.end(); ++it)
    {
        object_path /= *it;
    }
    object_name = object_path.string();
    return true;
}

bool isS3fqn(const fs::path& fqn) { return *(fqn.begin()) == "s3:"; }
