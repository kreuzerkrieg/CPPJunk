#pragma once

#include <aws/s3/S3Client.h>
#include <filesystem>


namespace fs = std::filesystem;

class S3File
{
public:
    explicit S3File(const std::string & fully_qualified_name);
    explicit S3File(const std::string & bucket, const Aws::S3::Model::Object & s3_object);
    explicit S3File(const fs::path & fully_qualified_name);

    S3File() = delete;
    S3File(const S3File &) = delete;
    S3File & operator=(const S3File &) = delete;

    S3File(S3File &&) = default;
    S3File & operator=(S3File &&) = default;

    ~S3File() = default;

    bool operator==(const S3File & rhs) const;
    bool operator!=(const S3File & rhs) const;

    bool exists();
    bool isFile();
    bool isDirectory();
    size_t size();
    size_t getSize() { return size(); }
    std::chrono::system_clock::time_point lastModifiedDate();
    void lastModifiedDate(const std::chrono::system_clock::time_point & new_last_modified);

    fs::path getFQName();
    std::string path();

    S3File copy(const std::string & new_name);

    S3File rename(const std::string & new_name);
    S3File rename(const fs::path & new_name);

    bool remove();
    bool remove(bool /*recursive*/);

    void print();

private:
    void validate();
    void setNotFound();
    S3File copyFile2File(const std::string & new_file_name);
    S3File copyFile2Folder(const std::string & new_folder_name);
    S3File copyFolder2Folder(const std::string & new_folder_name);

    Aws::S3::Model::ListObjectsV2OutcomeCallable lazy_result;
    fs::path fq_name;
    std::string bucket_name;
    std::string object_name;
    std::chrono::system_clock::time_point last_modification_date = std::chrono::system_clock::time_point::min();
    size_t file_size = 0;
    bool is_file = true;
    bool is_exist = false;
};
