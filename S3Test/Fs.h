#pragma once

#include <experimental/filesystem>

namespace fs = std::experimental::filesystem;

fs::path s3PathFixer(const std::string& path);

fs::path canonicalize(const fs::path& path);

bool S3fqn2parts(const fs::path& fqn, std::string& bucket_name, std::string& object_name);
