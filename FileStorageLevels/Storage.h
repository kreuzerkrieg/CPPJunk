#pragma once

#include <filesystem>
#include <memory>
#include <set>

struct Storage
{
    std::filesystem::path path;
    size_t priority;
};
struct StoragePriorityComp
{
    bool operator()(const Storage& lhs, const Storage& rhs) const { return lhs.priority < rhs.priority; }
};
using StorageContainer = std::set<Storage, StoragePriorityComp>;

struct Extent
{
    size_t offset = 0;
    size_t length = 0;
    size_t merge_level = 0; // For tracking only
};

inline bool operator<(size_t offset, const Extent& extent) { return offset < extent.offset; }
inline bool operator<(const Extent& extent, size_t offset) { return extent.offset < offset; }
inline bool operator<(const Extent& lhs, const Extent& rhs) { return lhs.offset < rhs.offset; }

using Extents = std::set<Extent, std::less<>>;

class IReadLeveledStorage
{
public:
    virtual ~IReadLeveledStorage() = default;
    virtual ssize_t read(void* buff, size_t buff_size) = 0;
    virtual off_t lseek(off_t offset, int whence) = 0;
};

class ReadLeveledStorage final : IReadLeveledStorage
{
public:
    struct Statistics
    {
        size_t primary_reads = 0;
        size_t secondary_reads = 0;
    };
    ReadLeveledStorage(const Storage& first_, const Storage& second_);
    ssize_t read(void* buff, size_t buff_size) override;
    off_t lseek(off_t offset, int whence) override;
    void printExtents() const;
    void printStats() const;

private:
    ssize_t update(void* buff, size_t buff_size);
    void mergeAdjacent(Extents::const_iterator it);
    void validateAdjacent(Extents::const_iterator it) const;

private:
    Extents extents{Extent{.offset = 0, .length = 0}};
    Statistics stats;
    int first_fd = -1;
    int second_fd = -1;
    size_t file_size = 0;
    size_t primed_file_size = 0;
    size_t position = 0;
    bool is_fully_primed = false;
};

class IWriteLeveledStorage
{
public:
    virtual ~IWriteLeveledStorage() = default;
    virtual ssize_t write(void* buff, size_t buff_size) = 0;
};

class WriteLeveledStorage : public IWriteLeveledStorage
{
public:
    ssize_t write(void* buff, size_t buff_size) override;
};

class LeveledStorage
{
private:
    std::unique_ptr<IReadLeveledStorage> reader;
    std::unique_ptr<IWriteLeveledStorage> writer;
};
