#pragma once

#include "Fs.h"
#include "S3LazyListRetriever.h"
#include <boost/uuid/nil_generator.hpp>
#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid.hpp>
#include <filesystem>
#include <iterator>

namespace fs = std::filesystem;

class S3DirectoryIterator
{
public:
    using value_type = fs::path;
    using difference_type = std::ptrdiff_t;
    using pointer = value_type*;
    using const_pointer = const value_type*;
    using reference = value_type&;
    using const_reference = const value_type&;
    using iterator_category = std::input_iterator_tag;
    using iterator = S3DirectoryIterator;
    using const_iterator = const S3DirectoryIterator;

    S3DirectoryIterator() = default;
    explicit S3DirectoryIterator(const fs::path& path);
    S3DirectoryIterator(const S3DirectoryIterator& rhs);
    S3DirectoryIterator(S3DirectoryIterator&& rhs) noexcept = delete;
    S3DirectoryIterator& operator=(const S3DirectoryIterator& rhs);
    S3DirectoryIterator& operator=(S3DirectoryIterator&& rhs) noexcept = delete;

    ~S3DirectoryIterator() = default;

    const_reference operator*() const;
    reference operator*();
    const_pointer operator->() const;
    pointer operator->();
    S3DirectoryIterator& operator++();

    iterator begin();
    const_iterator begin() const;
    const_iterator cbegin() const;
    iterator end();
    const_iterator end() const;
    const_iterator cend() const;

private:
    friend bool operator==(const S3DirectoryIterator& lhs, const S3DirectoryIterator& rhs);

    std::shared_ptr<S3LazyListRetriever> directory_retriever;
    std::vector<fs::path> directory_chunk;
    std::string bucket_name;
    std::string object_name;
    std::unique_ptr<S3DirectoryIterator> end_iterator;
    boost::uuids::uuid uuid{boost::uuids::nil_generator()()};
    const size_t page_size = 1000;
    size_t pos = 0;
};

inline S3DirectoryIterator begin(const S3DirectoryIterator& iter) { return std::cref(iter); }

inline S3DirectoryIterator end(S3DirectoryIterator) { return S3DirectoryIterator(); }

inline bool operator==(const S3DirectoryIterator& lhs, const S3DirectoryIterator& rhs) { return lhs.uuid == rhs.uuid; }

inline bool operator!=(const S3DirectoryIterator& lhs, const S3DirectoryIterator& rhs) { return !(lhs == rhs); }
