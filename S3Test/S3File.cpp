#include "S3File.h"
#include "Fs.h"
#include "S3LazyListRetriever.h"
#include <aws/s3/model/CopyObjectRequest.h>
#include <aws/s3/model/DeleteObjectRequest.h>
#include <aws/s3/model/DeleteObjectsRequest.h>
#include <aws/s3/model/ListObjectsV2Request.h>

S3File::S3File() {}
S3File::S3File(const std::string& fully_qualified_name) : S3File(fs::path(fully_qualified_name)) {}
S3File::S3File(const fs::path& fully_qualified_name) : fq_name(canonicalize(fully_qualified_name))
{
    if(!fqn2parts(fq_name, bucket_name, object_name))
    {
        throw std::runtime_error(fully_qualified_name.string() + " is not S3 fully qualified name.");
    }
    Aws::S3::Model::ListObjectsV2Request list_object_request;
    list_object_request.SetBucket(bucket_name.c_str());
    list_object_request.SetPrefix(object_name.c_str());
    list_object_request.SetDelimiter("/");
    list_object_request.SetMaxKeys(1);
    lazy_result = std::async(std::launch::async,
                             [list_object_request, s3_client{s3_client}]() { return s3_client.ListObjectsV2(list_object_request); });
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
void S3File::lastModifiedDate(const std::chrono::system_clock::time_point& new_last_modified) { validate(); }
fs::path S3File::getFQName() { return fq_name; }
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
        auto response = s3_client.DeleteObject(request);
        if(response.IsSuccess())
        {
            return true;
        }
        else
        {
            auto error = response.GetError();
            throw std::runtime_error((Aws::String("ERROR: ") + error.GetExceptionName() + ": " + error.GetMessage()).c_str());
        }
    }
    else
    {
        Aws::S3::Model::DeleteObjectsRequest request;
        Aws::S3::Model::Delete delete_object;
        S3LazyListRetriever retriever(bucket_name, object_name, false);
        auto list = retriever.next(16).get();

        while(!list.empty())
        {
            Aws::Vector<Aws::S3::Model::ObjectIdentifier> objects;
            std::transform(list.begin(), list.end(), std::back_inserter(objects), [](const auto& object) {
                Aws::S3::Model::ObjectIdentifier retVal;
                retVal.SetKey(object.c_str());
                return retVal;
            });
            Aws::S3::Model::Delete delete_list;
            delete_list.SetObjects(objects);
            request.SetBucket(bucket_name.c_str());
            request.SetDelete(delete_list);
            auto result = s3_client.DeleteObjects(request);
            if(result.IsSuccess())
            {
                list = retriever.next(16).get();
            }
            else
            {
                auto error = result.GetError();
                throw std::runtime_error((Aws::String("ERROR: ") + error.GetExceptionName() + ": " + error.GetMessage()).c_str());
            }
        }
    }
    setNotFound();
}
bool S3File::remove(bool) { remove(); }
void S3File::print()
{
    validate();
    std::cout << "Name: " << getFQName() << ", Size: " << size() << ", Is file: " << isFile() << ", exists: " << exists()
              << ", last mod time: " << lastModifiedDate().time_since_epoch().count() << std::endl;
}
bool S3File::fqn2parts(const fs::path& fqn, std::string& bucket_name, std::string& object_name)
{
    auto name = fqn;
    auto it = name.begin();
    if(*(it) != "s3:")
    {
        return false;
    }
    bool dot_removed = false;
    if(*(--name.end()) == ".")
    {
        name = name.parent_path();
        it = name.begin();
        dot_removed = true;
    }
    bucket_name = (*(++it)).string();
    ++it;

    fs::path object_path;
    for(; it != name.end(); ++it)
    {
        object_path /= *it;
    }
    object_name = object_path.string();
    if(dot_removed)
        object_name += fs::path::preferred_separator;
    return true;
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
            if(!result.GetContents().empty() /* && result.GetPrefix() == result.GetContents()[0].GetKey()*/)
            {
                //                is_file = true;
                file_size = result.GetContents()[0].GetSize();
                last_modification_date = result.GetContents()[0].GetLastModified().UnderlyingTimestamp();
            }
            else if(!result.GetCommonPrefixes().empty())
            {
                // is_file = false;
                file_size = 0;
            }
            else
            {
                setNotFound();
            }
        }
        else
        {
            setNotFound();
        }

        is_file = !fq_name.filename().empty() && fq_name.filename() != ".";
    }
}
void S3File::setNotFound()
{
    is_exist = false;
    is_file = true;
    file_size = 0;
    last_modification_date = std::chrono::system_clock::time_point::min();
}
S3File S3File::copyFile2File(const std::string& new_name)
{
    validate();

    std::string target_bucket;
    std::string target_key;
    if(!fqn2parts(new_name, target_bucket, target_key))
    {
        throw std::runtime_error(new_name + " is not S3 fully qualified name.");
    }
    auto source = (fs::path(bucket_name) / fs::path(object_name));
    Aws::S3::Model::CopyObjectRequest request;
    request.WithBucket(target_bucket.c_str()).WithKey(target_key.c_str()).WithCopySource(source.c_str());

    auto response = s3_client.CopyObject(request);
    if(response.IsSuccess())
    {
        return S3File(new_name);
    }
    else
    {
        auto error = response.GetError();
        throw std::runtime_error((Aws::String("ERROR: ") + error.GetExceptionName() + ": " + error.GetMessage()).c_str());
    }
}
S3File S3File::copyFile2Folder(const std::string& new_name)
{
    validate();

    std::string target_bucket;
    std::string target_key;
    if(!fqn2parts(new_name, target_bucket, target_key))
    {
        throw std::runtime_error(new_name + " is not S3 fully qualified name.");
    }

    auto source = (fs::path(bucket_name) / fs::path(object_name));
    auto file_name = fs::path(object_name).filename();
    auto target_path = fs::path(target_key) / file_name;
    target_key = target_path.string();

    Aws::S3::Model::CopyObjectRequest request;
    request.WithBucket(target_bucket.c_str()).WithKey(target_key.c_str()).WithCopySource(source.c_str());

    auto response = s3_client.CopyObject(request);
    if(response.IsSuccess())
    {
        return S3File(new_name);
    }
    else
    {
        auto error = response.GetError();
        throw std::runtime_error((Aws::String("ERROR: ") + error.GetExceptionName() + ": " + error.GetMessage()).c_str());
    }
}
S3File S3File::copyFolder2Folder(const std::string& new_name)
{
    validate();

    S3LazyListRetriever retriever(bucket_name, object_name, false);
    auto list = retriever.next(16).get();

    while(!list.empty())
    {
        for(const auto& file : list)
        {
            std::string key_name = file.c_str();
            key_name.erase(0, object_name.length());
            fs::path new_fqn = "s3:/";
            new_fqn /= bucket_name;
            new_fqn /= file.c_str();
            S3File tmp_file(new_fqn);
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

                fs::path full_name(file.c_str());
                auto it = full_name.begin();
                std::advance(it, prefix_length);

                fs::path full_target_name = new_name;
                for(; it != full_name.end(); ++it)
                {
                    full_target_name /= *it;
                }
                tmp_file.copy(full_target_name);
            }
        }
        list = retriever.next(16).get();
    }

    return S3File(new_name);
}
