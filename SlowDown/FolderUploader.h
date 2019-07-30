#pragma once
#include <string>

class FolderUploader
{
public:
    FolderUploader() = delete;
    explicit FolderUploader(const std::string& local_path, const std::string& s3_fqp);
    void Upload();

private:
    std::string local_path;
    std::string s3_fqp;
    const size_t max_buffers;
    const size_t part_size;
    size_t buffers_inflight = 0;
};
