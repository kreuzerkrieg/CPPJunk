#include "S3File.h"
#include "S3DirectoryIterator.h"
#include "Utils.h"
#include <aws/s3/model/CopyObjectRequest.h>
#include <aws/s3/model/DeleteObjectRequest.h>
#include <aws/s3/model/DeleteObjectsRequest.h>
#include <aws/s3/model/HeadObjectRequest.h>
#include <aws/s3/model/ListObjectsV2Request.h>

S3File::S3File(const std::string& fully_qualified_name) : S3File(s3canonicalize(fully_qualified_name)) {}
S3File::S3File(const fs::path& fully_qualified_name) : fq_name(s3canonicalize(fully_qualified_name))
{
    if(!s3fqn2parts(fq_name, bucket_name, object_name))
    {
        throw std::runtime_error(fully_qualified_name.string() + " is not S3 fully qualified name.");
    }
    Aws::S3::Model::ListObjectsV2Request list_object_request;
    list_object_request.SetBucket(bucket_name.c_str());
    if(object_name.at(object_name.length() - 1) == '/')
        list_object_request.SetPrefix(object_name.substr(0, object_name.length() - 1).c_str());
    else
        list_object_request.SetPrefix(object_name.c_str());
    list_object_request.SetDelimiter("/");
    list_object_request.SetMaxKeys(1);
    lazy_result = getS3Client().ListObjectsV2Callable(list_object_request);
}
S3File::S3File(const std::string& bucket, const Aws::S3::Model::Object& s3_object)
{
    is_exist = true;
    is_file = true;
    object_name = s3_object.GetKey();
    bucket_name = bucket;
    fq_name = parts2s3fqn(bucket_name, object_name);
    file_size = s3_object.GetSize();
    last_modification_date = s3_object.GetLastModified().UnderlyingTimestamp();
}
bool S3File::operator==(const S3File& rhs) const { return fq_name == rhs.fq_name; }
bool S3File::operator!=(const S3File& rhs) const { return !(*this == rhs); }
bool S3File::exists()
{
    validate();
    return is_exist;
}
bool S3File::isFile()
{
    validate();
    return is_file;
}
bool S3File::isDirectory() { return !is_file; }

size_t S3File::size()
{
    validate();
    return file_size;
}
std::chrono::system_clock::time_point S3File::lastModifiedDate()
{
    validate();
    return last_modification_date;
}
void S3File::lastModifiedDate(const std::chrono::system_clock::time_point& /*new_last_modified*/) { validate(); }
fs::path S3File::getFQName() { return fq_name; }
std::string S3File::path() { return fq_name.string(); }
S3File S3File::copy(const std::string& new_name)
{
    bool is_this_file = isFile();
    bool is_that_file = S3File(new_name).isFile();
    if(is_this_file)
    {
        if(is_that_file)
        {
            return copyFile2File(new_name);
        }
        else
        {
            return copyFile2Folder(new_name);
        }
    }
    else
    {
        if(is_that_file)
        {
            throw std::runtime_error("Trying to copy folder to file");
        }
        else
        {
            return copyFolder2Folder(new_name);
        }
    }
}
S3File S3File::rename(const std::string& new_name)
{
    auto ret_val = copy(new_name);
    remove();
    return ret_val;
}
S3File S3File::rename(const fs::path& new_name)
{
    validate();
    return rename(new_name.string());
}
bool S3File::remove()
{
    validate();
    if(is_file)
    {
        Aws::S3::Model::DeleteObjectRequest request;
        request.WithBucket(bucket_name.c_str()).WithKey(object_name.c_str());
        auto response = getS3Client().DeleteObject(request);
        if(response.IsSuccess())
        {
            return true;
        }
        else
        {
            const auto& error = response.GetError();
            throw std::runtime_error((Aws::String("Exception thrown for '") + getFQName().c_str() + "' - " + error.GetExceptionName() +
                                      ": " + error.GetMessage())
                                         .c_str());
        }
    }
    else
    {
        auto delete_thread = std::thread([fqn{fq_name}, bucket{bucket_name}]() {
            Aws::S3::Model::DeleteObjectsRequest request;
            Aws::S3::Model::Delete delete_list;

            for(auto& item : S3DirectoryIterator(fqn))
            {
                Aws::S3::Model::ObjectIdentifier object;
                std::string bn;
                std::string on;
                s3fqn2parts(item, bn, on);
                object.SetKey(on.c_str());
                delete_list.AddObjects(std::move(object));
            }

            request.SetBucket(bucket.c_str());
            request.SetDelete(delete_list);
            auto result = getS3Client().DeleteObjects(request);
            if(!result.IsSuccess())
            {
                const auto& error = result.GetError();
                throw std::runtime_error(
                    (Aws::String("Exception thrown for '") + fqn.c_str() + "' - " + error.GetExceptionName() + ": " + error.GetMessage())
                        .c_str());
            }
        });
        delete_thread.join();
    }
    setNotFound();
    return true;
}
bool S3File::remove(bool) { return remove(); }
void S3File::print()
{
    validate();
    std::cout << "Name: " << getFQName() << ", Size: " << size() << ", Is file: " << isFile() << ", exists: " << exists()
              << ", last mod time: " << lastModifiedDate().time_since_epoch().count() << std::endl;
}

void S3File::validate()
{
    if(__glibc_unlikely(lazy_result.valid()))
    {
        auto outcome = lazy_result.get();
        if(outcome.IsSuccess())
        {
            is_exist = true;
            auto result = outcome.GetResultWithOwnership();
            if(!result.GetContents().empty())
            {
                file_size = result.GetContents()[0].GetSize();
                last_modification_date = result.GetContents()[0].GetLastModified().UnderlyingTimestamp();
                is_file = true;
            }
            else if(!result.GetCommonPrefixes().empty())
            {
                file_size = 0;
                is_file = false;
                if(object_name.at(object_name.length() - 1) != '/')
                    fq_name = s3canonicalize(fq_name.string() + "/");
            }
            else
            {
                setNotFound();
            }
        }
        else
        {
            setNotFound();
            const auto& error = outcome.GetError();
            throw std::runtime_error((Aws::String("Exception thrown for '") + getFQName().c_str() + "' - " + error.GetExceptionName() +
                                      ": " + error.GetMessage())
                                         .c_str());
        }

        //        is_file = !fq_name.filename().empty() && fq_name.filename() != ".";
    }
}
void S3File::setNotFound()
{
    is_exist = false;
    is_file = true;
    file_size = 0;
    last_modification_date = std::chrono::system_clock::time_point::min();
}
S3File S3File::copyFile2File(const std::string& new_file_name)
{
    validate();

    std::string target_bucket;
    std::string target_key;
    if(!s3fqn2parts(new_file_name, target_bucket, target_key))
    {
        throw std::runtime_error(new_file_name + " is not S3 fully qualified name.");
    }
    auto source = (fs::path(bucket_name) / fs::path(object_name));
    Aws::S3::Model::CopyObjectRequest request;
    request.WithBucket(target_bucket.c_str()).WithKey(target_key.c_str()).WithCopySource(source.c_str());

    auto response = getS3Client().CopyObject(request);
    if(response.IsSuccess())
    {
        return S3File(new_file_name);
    }
    else
    {
        const auto& error = response.GetError();
        throw std::runtime_error((Aws::String("Exception thrown for '") + getFQName().c_str() + "' - " + error.GetExceptionName().c_str() +
                                  ": " + error.GetMessage())
                                     .c_str());
    }
}
S3File S3File::copyFile2Folder(const std::string& new_folder_name)
{
    validate();

    std::string target_bucket;
    std::string target_key;
    if(!s3fqn2parts(new_folder_name, target_bucket, target_key))
    {
        throw std::runtime_error(new_folder_name + " is not S3 fully qualified name.");
    }

    auto source = (fs::path(bucket_name) / fs::path(object_name));
    auto file_name = fs::path(object_name).filename();
    auto target_path = fs::path(target_key) / file_name;
    target_key = target_path.string();

    Aws::S3::Model::CopyObjectRequest request;
    request.WithBucket(target_bucket.c_str()).WithKey(target_key.c_str()).WithCopySource(source.c_str());

    auto response = getS3Client().CopyObject(request);
    if(response.IsSuccess())
    {
        return S3File(new_folder_name);
    }
    else
    {
        const auto& error = response.GetError();
        throw std::runtime_error((Aws::String("Exception thrown for '") + getFQName().c_str() + "' - " + error.GetExceptionName().c_str() +
                                  ": " + error.GetMessage())
                                     .c_str());
    }
}
S3File S3File::copyFolder2Folder(const std::string& new_folder_name)
{
    validate();
    //    ThreadPool pool(64);
    for(auto& item : S3DirectoryIterator(fq_name))
    {
        //        pool.schedule([&]() {
        S3File tmp_file(item);
        if(tmp_file != *this)
        {
            fs::path prefix(object_name);
            auto prefix_it = prefix.begin();
            auto prefix_length = std::distance(prefix_it, prefix.end());
            std::advance(prefix_it, prefix_length - 1);
            if(*prefix_it == ".")
            {
                --prefix_length;
            }

            auto it = item.begin();
            // advance the iterator by calculated length and additional two positions - one for "s3:" prefix and one for bucket name
            std::advance(it, prefix_length + 2);

            fs::path full_target_name = new_folder_name;
            for(; it != item.end(); ++it)
            {
                full_target_name /= *it;
            }
            tmp_file.copy(s3canonicalize(full_target_name));
        }
        //        });
    }
    //    pool.wait();

    return S3File(new_folder_name);
}
