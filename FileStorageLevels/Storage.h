#pragma once

#include <boost/serialization/serialization.hpp>
#include <boost/uuid/uuid.hpp>
#include <cstring>
#include <filesystem>
#include <memory>
#include <set>
#include <thread>

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
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int /*version*/)
    {
        ar& offset& length& merge_level;
    }
};

inline bool operator==(const Extent& lhs, const Extent& rhs)
{
    return std::tie(lhs.offset, lhs.length, lhs.merge_level) == std::tie(rhs.offset, rhs.length, rhs.merge_level);
}
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

template <typename T, std::enable_if_t<std::is_arithmetic_v<T> && std::is_signed_v<T>>* = nullptr>
void checkAndThrow(T result)
{
    if(result == -1)
    {
        throw std::runtime_error(std::strerror(errno));
    }
}

class ReadLeveledStorage final : IReadLeveledStorage
{
public:
    struct Statistics
    {
        size_t primary_reads = 0;
        size_t secondary_reads = 0;
    };
    ReadLeveledStorage(const Storage& first_, const Storage& second_);
    ~ReadLeveledStorage();
    ssize_t read(void* buff, size_t buff_size) override;
    off_t lseek(off_t offset, int whence) override;
    void printExtents() const;
    void printStats() const;

private:
    ssize_t update(void* buff, size_t buff_size);
    void mergeAdjacent(Extents::const_iterator it);
    void validateAdjacent(Extents::const_iterator it) const;

    void saveExtentFile();
    bool loadExtentFile();
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int /*version*/)
    {
        ar& is_fully_primed& primed_file_size& extents& signature;
    }

private:
    static inline Extent dummy{Extent{.offset = 0, .length = 0}};
    Extents extents{dummy};
    Statistics stats;
    std::thread extent_thread;
    const std::string sig_str = "bb281707-9da9-4bff-99f2-5b55efd3fff1";
    boost::uuids::uuid signature;
    std::filesystem::path extfile_name;
    size_t file_size = 0;
    size_t primed_file_size = 0;
    size_t position = 0;
    int first_fd = -1;
    int second_fd = -1;
    bool is_fully_primed = false;
    bool shutting_down = false;
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
