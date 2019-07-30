#pragma once

#include <aws/s3/S3Client.h>
#include <deque>
#include <filesystem>
namespace fs = std::filesystem;

class Uploader
{
public:
    Uploader();
    void upload(const fs::path& fully_qualified_name, const fs::path& source_file);

private:
    void uploadThread(const fs::path& fully_qualified_name);

    const size_t part_size = 10 * 1024 * 1024;
    bool abort = false;
};
