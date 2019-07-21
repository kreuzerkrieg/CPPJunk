#pragma once

#include "Fs.h"
#include "S3File.h"
#include "S3LazyListRetriever.h"
#include <boost/uuid/nil_generator.hpp>
#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid.hpp>
#include <experimental/filesystem>
#include <iterator>

namespace fs = std::experimental::filesystem;

class DirectoryIterator
{
public:
    using value_type = S3File;
    using difference_type = std::ptrdiff_t;
    using pointer = const S3File*;
    using reference = const S3File&;
    using iterator_category = std::input_iterator_tag;

    DirectoryIterator() = default;
    explicit DirectoryIterator(const fs::path& path)
    {
        if(!S3fqn2parts(path, bucket_name, object_name))
        {
            throw std::runtime_error(path.string() + " is not S3 fully qualified name.");
        }
        uuid = boost::uuids::random_generator()();
        end_iterator = std::make_unique<DirectoryIterator>();

        directory_retriever = std::make_shared<S3LazyListRetriever>(bucket_name, object_name, false);
        auto list = directory_retriever->next(128).get();
        std::transform(list.begin(), list.end(), std::back_inserter(directory_chunk), [this](const auto& object) {
            fs::path new_fqn = "s3:/";
            new_fqn /= bucket_name;
            new_fqn /= object.c_str();
            return new_fqn;
        });
        if(directory_chunk.empty())
        {
            *this = *end_iterator;
        }
    }
    DirectoryIterator(const DirectoryIterator& rhs) { *this = rhs; }
    DirectoryIterator(DirectoryIterator&& rhs) noexcept = delete;
    DirectoryIterator& operator=(const DirectoryIterator& rhs)
    {
        if(!rhs.uuid.is_nil())
        {
            directory_retriever = std::make_shared<S3LazyListRetriever>(*rhs.directory_retriever);
            directory_chunk = rhs.directory_chunk;
            bucket_name = rhs.bucket_name;
            object_name = rhs.object_name;
            end_iterator = std::make_unique<DirectoryIterator>();
            pos = rhs.pos;
            uuid = rhs.uuid;
        }
        else
            uuid = boost::uuids::uuid(boost::uuids::nil_generator()());
        return *this;
    }
    DirectoryIterator& operator=(DirectoryIterator&& rhs) noexcept = delete;

    ~DirectoryIterator() = default;

    const fs::path& operator*() const { return directory_chunk.at(pos); }
    fs::path& operator*() { return directory_chunk.at(pos); }
    const fs::path* operator->() const { return &directory_chunk.at(pos); }
    fs::path* operator->() { return &directory_chunk.at(pos); }
    DirectoryIterator& operator++()
    {
        ++pos;
        if(pos == directory_chunk.size())
        {
            auto list = directory_retriever->next(128).get();
            if(list.empty())
            {
                --pos;
                *this = *end_iterator;
                return *this;
            }
            pos = 0;
            directory_chunk.clear();
            std::transform(list.begin(), list.end(), std::back_inserter(directory_chunk), [this](const auto& object) {
                fs::path new_fqn = "s3:/";
                new_fqn /= bucket_name;
                new_fqn /= object.c_str();
                return new_fqn;
            });
        }
        std::vector<int> v;
        return *this;
    }

private:
    friend bool operator==(const DirectoryIterator& lhs, const DirectoryIterator& rhs);
    std::shared_ptr<S3LazyListRetriever> directory_retriever;
    std::vector<fs::path> directory_chunk;
    std::string bucket_name;
    std::string object_name;
    std::unique_ptr<DirectoryIterator> end_iterator;
    boost::uuids::uuid uuid{boost::uuids::nil_generator()()};
    size_t pos = 0;
};

inline DirectoryIterator begin(DirectoryIterator iter) { return std::cref(iter); }

inline DirectoryIterator end(DirectoryIterator) { return DirectoryIterator(); }

inline bool operator==(const DirectoryIterator& lhs, const DirectoryIterator& rhs) { return lhs.uuid == rhs.uuid; }

inline bool operator!=(const DirectoryIterator& lhs, const DirectoryIterator& rhs) { return !(lhs == rhs); }
