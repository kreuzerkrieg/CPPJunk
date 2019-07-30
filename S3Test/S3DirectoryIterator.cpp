#include "S3DirectoryIterator.h"
#include "S3LazyListRetriever.h"
#include "Utils.h"
#include "S3File.h"

S3DirectoryIterator::S3DirectoryIterator(const fs::path & path)
{
    if (!s3fqn2parts(path, bucket_name, object_name))
    {
        throw std::runtime_error(path.string() + " is not S3 fully qualified name.");
    }

    end_iterator = std::make_unique<S3DirectoryIterator>();
    uuid = boost::uuids::random_generator()();

    directory_retriever = std::make_shared<S3LazyListRetriever>(bucket_name, object_name, false);
    auto list = directory_retriever->next(page_size).get();
    std::transform(list.begin(), list.end(), std::back_inserter(directory_chunk), [](auto & object) {
        return object.getFQName();
    });
    if (directory_chunk.empty())
    {
        *this = *end_iterator;
    }
}

S3DirectoryIterator::S3DirectoryIterator(const S3DirectoryIterator & rhs)
{
    *this = rhs;
}
S3DirectoryIterator & S3DirectoryIterator::operator=(const S3DirectoryIterator & rhs)
{
    if (!rhs.uuid.is_nil())
    {
        directory_retriever = std::make_shared<S3LazyListRetriever>(*rhs.directory_retriever);
        directory_chunk = rhs.directory_chunk;
        bucket_name = rhs.bucket_name;
        object_name = rhs.object_name;
        end_iterator = std::make_unique<S3DirectoryIterator>();
        pos = rhs.pos;
        uuid = rhs.uuid;
    }
    else
        uuid = boost::uuids::uuid(boost::uuids::nil_generator()());
    return *this;
}
S3DirectoryIterator::const_reference & S3DirectoryIterator::operator*() const
{
    return directory_chunk.at(pos);
}

S3DirectoryIterator::reference S3DirectoryIterator::operator*()
{
    return directory_chunk.at(pos);
}
S3DirectoryIterator::const_pointer S3DirectoryIterator::operator->() const
{
    return &directory_chunk.at(pos);
}
S3DirectoryIterator::pointer S3DirectoryIterator::operator->()
{
    return &directory_chunk.at(pos);
}
S3DirectoryIterator & S3DirectoryIterator::operator++()
{
    ++pos;
    if (pos == directory_chunk.size())
    {
        auto list = directory_retriever->next(page_size).get();
        if (list.empty())
        {
            --pos;
            *this = *end_iterator;
            return *this;
        }
        pos = 0;
        directory_chunk.clear();
        std::transform(list.begin(), list.end(), std::back_inserter(directory_chunk), [](auto & object) {
            return object.getFQName();
        });
    }
    return *this;
}
S3DirectoryIterator::iterator S3DirectoryIterator::begin()
{
    return *this;
}
S3DirectoryIterator::const_iterator S3DirectoryIterator::begin() const
{
    return *this;
}
S3DirectoryIterator::const_iterator S3DirectoryIterator::cbegin() const
{
    return *this;
}
S3DirectoryIterator::iterator S3DirectoryIterator::end()
{
    return *end_iterator;
}
S3DirectoryIterator::const_iterator S3DirectoryIterator::end() const
{
    return *end_iterator;
}
S3DirectoryIterator::const_iterator S3DirectoryIterator::cend() const
{
    return *end_iterator;
}
