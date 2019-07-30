#include "Fs.h"

fs::path s3PathFixer(const std::string& path)
{
    fs::path ret_val;
    if(path.length() > 3 && path.substr(0, 4) != "s3:/")
    {
        ret_val = "s3://dev-shadow";
    }
    ret_val /= path;
    return ret_val;
}

fs::path canonicalize(const fs::path& path)
{
    bool is_folder = false;
    if(*(--path.end()) == ".")
        is_folder = true;
    auto it = path.begin();
    std::string bucket;
    if(*it == "s3:")
    {
        ++it; // skip "s3://"
        bucket = *it;
        ++it; // skip bucket
    }
    fs::path clean;
    for(; it != path.end(); ++it)
    {
        if(*it == ".")
        {
            continue;
        }
        if(*it == "..")
        {
            clean = clean.parent_path();
            continue;
        }
        clean /= *it;
    }

    if(is_folder)
    {
        clean /= ".";
    }
    fs::path ret_val = "s3://";
    ret_val /= bucket;
    ret_val /= clean;
    return ret_val;
}

bool S3fqn2parts(const fs::path& fqn, std::string& bucket_name, std::string& object_name)
{
    auto name = canonicalize(fqn);
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
