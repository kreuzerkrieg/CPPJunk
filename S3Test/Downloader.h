#pragma once

#include <aws/s3/S3Client.h>
#include <deque>
#include <filesystem>

namespace fs = std::filesystem;

class FileDownloader
{
public:
    void download(const fs::path& fully_qualified_name, const fs::path& target_file);

private:
	void downloadThread(const fs::path& fully_qualified_name);

    Aws::S3::S3Client s3_client;
    std::deque<Aws::S3::Model::GetObjectOutcomeCallable> part_completion;
    std::thread downloading_thread;
    std::condition_variable cv;
    std::mutex download_mtx;
    std::mutex download_in_progress;
    bool has_work_to_do = true;
    bool abort = false;
};