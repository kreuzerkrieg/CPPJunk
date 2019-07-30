#pragma once
#include <filesystem>
#include <string>

constexpr uint64_t operator"" _KiB(unsigned long long int bytes) { return bytes * 1024; }

constexpr uint64_t operator"" _MiB(unsigned long long int bytes) { return bytes * 1024_KiB; }

constexpr uint64_t operator"" _GiB(unsigned long long int bytes) { return bytes * 1024_MiB; }

constexpr uint64_t operator"" _TiB(unsigned long long int bytes) { return bytes * 1024_GiB; }

constexpr uint64_t operator"" _KB(unsigned long long int bytes) { return bytes * 1000; }

constexpr uint64_t operator"" _MB(unsigned long long int bytes) { return bytes * 1000_KB; }

constexpr uint64_t operator"" _GB(unsigned long long int bytes) { return bytes * 1000_MB; }

constexpr uint64_t operator"" _TB(unsigned long long int bytes) { return bytes * 1000_GB; }

namespace Aws::S3
{
class S3Client;
}

namespace fs = std::filesystem;

Aws::S3::S3Client& getS3Client();
Aws::S3::S3Client& getS3Client(const std::string& access_key, const std::string& secret_key);
fs::path s3canonicalize(const fs::path& path);
fs::path parts2s3fqn(const std::string& bucket_name, const std::string& object_name);
bool s3fqn2parts(const fs::path& fqn, std::string& bucket_name, std::string& object_name);
bool isS3fqn(const fs::path& fqn);
