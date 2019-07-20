#pragma once

#include "Fs.h"
#include "S3File.h"
#include "S3LazyListRetriever.h"
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
    explicit DirectoryIterator(const fs::path& path) : directory_chunk(std::make_shared<std::vector<S3File>>())
    {
        if(!S3fqn2parts(path, bucket_name, object_name))
        {
            throw std::runtime_error(path.string() + " is not S3 fully qualified name.");
        }
        directory_retriever = std::make_shared<S3LazyListRetriever>(bucket_name, object_name);
        auto list = directory_retriever->next(128).get();
        std::transform(list.begin(), list.end(), std::back_inserter(*directory_chunk), [this](const auto& object) {
            fs::path new_fqn = "s3:/";
            new_fqn /= bucket_name;
            new_fqn /= object.GetKey();
            return S3File(new_fqn);
        });
        if(directory_chunk->empty())
            *this = DirectoryIterator();
        end_iterator = std::make_shared<DirectoryIterator>();
    }
    DirectoryIterator(const DirectoryIterator& rhs) = default;
    DirectoryIterator(DirectoryIterator&& rhs) noexcept = default;
    DirectoryIterator& operator=(const DirectoryIterator& rhs) = default;
    DirectoryIterator& operator=(DirectoryIterator&& rhs) noexcept = default;

    ~DirectoryIterator() = default;

    const S3File& operator*() const { return directory_chunk->at(pos); }
    S3File& operator*() { return directory_chunk->at(pos); }
    const S3File* operator->() const { return &directory_chunk->at(pos); }
    S3File* operator->() { return &directory_chunk->at(pos); }
    DirectoryIterator& operator++()
    {
        ++pos;
        if(pos == directory_chunk->size())
        {
            auto list = directory_retriever->next(128).get();
            if(list.empty())
            {
                --pos;
                return *end_iterator;
            }
            pos = 0;
            directory_chunk->clear();
            std::transform(list.begin(), list.end(), std::back_inserter(*directory_chunk), [this](const auto& object) {
                fs::path new_fqn = "s3:/";
                new_fqn /= bucket_name;
                new_fqn /= object.GetKey();
                return S3File(new_fqn);
            });
        }
        return *this;
    }

private:
    friend bool operator==(const DirectoryIterator& lhs, const DirectoryIterator& rhs);
    std::shared_ptr<S3LazyListRetriever> directory_retriever;
    std::shared_ptr<std::vector<S3File>> directory_chunk;
    std::string bucket_name;
    std::string object_name;
    std::shared_ptr<DirectoryIterator> end_iterator;
    size_t pos = 0;
};

inline DirectoryIterator begin(DirectoryIterator iter) noexcept { return iter; }

inline DirectoryIterator end(DirectoryIterator) noexcept { return DirectoryIterator(); }

inline bool operator==(const DirectoryIterator& lhs, const DirectoryIterator& rhs)
{
    return !rhs.directory_retriever.owner_before(lhs.directory_retriever) && !lhs.directory_retriever.owner_before(rhs.directory_retriever);
}

inline bool operator!=(const DirectoryIterator& lhs, const DirectoryIterator& rhs) { return !(lhs == rhs); }
