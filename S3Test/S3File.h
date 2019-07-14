#pragma once

#include <aws/s3/S3Client.h>
#include <experimental/filesystem>

namespace fs = std::experimental::filesystem;

class S3File
{
public:
    S3File();
    explicit S3File(const std::string& fully_qualified_name);
    explicit S3File(const fs::path& fully_qualified_name);
    bool operator==(const S3File& rhs) const;
    bool operator!=(const S3File& rhs) const;
    bool exists();
    bool isFile();
    size_t size();
    std::chrono::system_clock::time_point lastModifiedDate();
    void lastModifiedDate(const std::chrono::system_clock::time_point& new_last_modified);

    fs::path getFQName();

    S3File copy(const std::string& new_name);

    S3File rename(const std::string& new_name);
    S3File rename(const fs::path& new_name);

    bool remove();
    bool remove(bool /*recursive*/);

    void print();

private:
    static bool fqn2parts(const fs::path& fqn, std::string& bucket_name, std::string& object_name);
    void validate();
    void setNotFound();
    S3File copyFile2File(const std::string& new_name);
    S3File copyFile2Folder(const std::string& new_name);
    S3File copyFolder2Folder(const std::string& new_name);

    Aws::S3::S3Client s3_client;
    Aws::S3::Model::ListObjectsV2OutcomeCallable lazy_result;
    fs::path fq_name;
    std::string bucket_name;
    std::string object_name;
    std::chrono::system_clock::time_point last_modification_date = std::chrono::system_clock::time_point::min();
    size_t file_size = 0;
    bool is_file = true;
    bool is_exist = false;
};