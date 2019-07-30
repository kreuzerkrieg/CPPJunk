#pragma once

#include <aws/s3/S3Client.h>
#include <deque>
#include <filesystem>

namespace fs = std::filesystem;

class FileDownloaderV2
{
public:
    void download(const fs::path& fully_qualified_name, const fs::path& target_file) const;
    void abort();

private:
    void popAndSave(std::deque<Aws::S3::Model::GetObjectOutcomeCallable>& completion_queue, std::ofstream& file) const;

    bool abort_ops = false;
};