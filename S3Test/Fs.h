#pragma once

#include <filesystem>

namespace fs = std::filesystem;

fs::path s3PathFixer(const std::string& path);

fs::path canonicalize(const fs::path& path);

bool S3fqn2parts(const fs::path& fqn, std::string& bucket_name, std::string& object_name);

inline fs::path parts2s3fqn(const std::string& bucket_name, const std::string& object_name)
{
    fs::path tmp = "s3://";
    tmp /= bucket_name;
    tmp /= object_name;
    return canonicalize(tmp);
}
