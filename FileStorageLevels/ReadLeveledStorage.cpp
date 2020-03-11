#include "Storage.h"
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <unistd.h>

template <typename T, std::enable_if_t<std::is_arithmetic_v<T> && std::is_signed_v<T>>* = nullptr>
void checkAndThrow(T result)
{
    if(result == -1)
    {
        throw std::runtime_error(std::strerror(errno));
    }
}

ReadLeveledStorage::ReadLeveledStorage(const Storage& first_, const Storage& second_)
{
    file_size = std::filesystem::file_size(second_.path);
    first_fd = open(first_.path.c_str(), O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    checkAndThrow(first_fd);
    second_fd = open(second_.path.c_str(), O_RDONLY);
    checkAndThrow(second_fd);
    auto res = ftruncate(first_fd, file_size);
    checkAndThrow(res);
}

ssize_t ReadLeveledStorage::read(void* buff, size_t buff_size)
{
    if(buff_size == 0)
        return 0;
    if(is_fully_primed)
    {
        ++stats.primary_reads;
        return ::read(first_fd, buff, buff_size);
    }

    ssize_t ret_val = 0;
    while(buff_size > 0)
    {
        auto extent = extents.lower_bound(position);
        // The easy case - requested extent is beyond the end of the extent we have at hand, go ahead and read it.
        if(extent == extents.end())
        {
            auto last = std::prev(extent);
            if(last->offset + last->length <= position)
            {
                ret_val += update(buff, buff_size);
                return ret_val;
            }
            // Edge case, part of last extent covers part of requested data
            extent = last;
        }
        if(extent->offset > position)
        {
            --extent;
        }

        if(position >= extent->offset && position < extent->offset + extent->length)
        {
            // Have a piece in extent, read it to the end of extent
            auto bytes_to_read = std::min(buff_size, extent->length - (position - extent->offset));
            auto res = ::read(first_fd, buff, bytes_to_read);
            ++stats.primary_reads;
            checkAndThrow(res);
            buff += res;
            position += res;
            ::lseek(second_fd, position, SEEK_SET);
            ret_val += res;
            buff_size -= res;
        }
        else
        {
            // "position" situated at hole
            auto next = std::next(extent);

            // Easy case, the asked buffer ends before next extent starts, read it all, or we've reached the last extent, read it all.
            if(next == extents.end() || position + buff_size < next->offset)
            {
                ret_val += update(buff, buff_size);
                return ret_val;
            }
            // Less convenient case, the buffer ends somewhere beyond the next extent start, read to start and roll
            auto res = update(buff, next->offset - position);
            buff += res;
            ret_val += res;
            buff_size -= res;
        }
    }
    return ret_val;
}

off_t ReadLeveledStorage::lseek(off_t offset, int whence)
{
    auto new_position = position;
    switch(whence)
    {
    case SEEK_SET:
        new_position = offset;
        break;
    case SEEK_CUR:
        new_position += offset;
        break;
    case SEEK_END:
        new_position = file_size + offset;
        break;
    default:
        throw std::runtime_error("Unknown SEEK mode " + std::to_string(whence));
    }

    if(new_position < 0)
    {
        throw std::runtime_error("Invalid argument. Offset: " + std::to_string(offset) + " whence: " + std::to_string(whence));
    }
    position = new_position;
    ::lseek(first_fd, position, SEEK_SET);
    ::lseek(second_fd, position, SEEK_SET);
    return position;
}

ssize_t ReadLeveledStorage::update(void* buff, size_t buff_size)
{
    auto read_res = ::read(second_fd, buff, buff_size);
    ++stats.secondary_reads;
    checkAndThrow(read_res);
    if(read_res == 0)
    {
        return 0;
    }

    auto write_res = ::write(first_fd, buff, read_res);
    checkAndThrow(write_res);
    if(read_res != write_res)
    {
        throw std::logic_error("Read/write failed. The read operation yield " + std::to_string(read_res) +
                               "bytes, but the write operation written " + std::to_string(write_res) + "bytes.");
    }
    if(position == 0 && extents.begin()->offset == 0 && extents.begin()->length == 0)
    {
        // delete convenience placeholder, since we have real data to populate it with.
        extents.erase(extents.begin());
    }

    auto emplaced = extents.emplace(Extent{.offset = position, .length = static_cast<size_t>(write_res)});
    if(!emplaced.second)
    {
        throw std::logic_error("Failed to emplace new extent - offset: " + std::to_string(emplaced.first->offset) +
                               ", length: " + std::to_string(emplaced.first->length));
    }
    mergeAdjacent(emplaced.first);
    position += write_res;
    primed_file_size += write_res;
    is_fully_primed = primed_file_size == file_size;
    if(primed_file_size > file_size)
    {
        //        throw std::logic_error("Current primary file size " + std::to_string(primed_file_size) + " is greater than secondary file
        //        size " +
        //                               std::to_string(file_size));
    }
    if(is_fully_primed)
    {
        extents.clear();
        extents.emplace(Extent{.offset = 0, .length = file_size});
    }
    return write_res;
}

void ReadLeveledStorage::mergeAdjacent(Extents::const_iterator it)
{
    validateAdjacent(it);
    {
        auto prev = std::prev(it);
        if(prev != it && prev->offset + prev->length == it->offset)
        {
            Extent new_extent;
            new_extent.offset = prev->offset;
            new_extent.length = prev->length + it->length;
            new_extent.merge_level = prev->merge_level + it->merge_level + 1;
            extents.erase(prev);
            extents.erase(it);
            auto emplaced = extents.emplace(new_extent);
            if(!emplaced.second)
            {
                throw std::logic_error("Failed to emplace new extent - offset: " + std::to_string(emplaced.first->offset) +
                                       ", length: " + std::to_string(emplaced.first->length));
            }
            it = emplaced.first;
        }
    }

    {
        auto next = std::next(it);
        if(next != extents.end() && it->offset + it->length == next->offset)
        {
            Extent new_extent;
            new_extent.offset = it->offset;
            new_extent.length = it->length + next->length;
            new_extent.merge_level = next->merge_level + it->merge_level + 1;
            extents.erase(next);
            extents.erase(it);
            auto emplaced = extents.emplace(new_extent);
            if(!emplaced.second)
            {
                throw std::logic_error("Failed to emplace new extent - offset: " + std::to_string(emplaced.first->offset) +
                                       ", length: " + std::to_string(emplaced.first->length));
            }
        }
    }
}

void ReadLeveledStorage::validateAdjacent(Extents::const_iterator it) const
{
    auto prev = std::prev(it);
    if(prev != it && prev->offset + prev->length > it->offset)
    {
        throw std::logic_error("Invalid adjacent extents found! Extent with offset " + std::to_string(prev->offset) + " and length " +
                               std::to_string(prev->length) + " tresspassing into next extent");
    }

    auto next = std::next(it);
    if(next != extents.end() && it->offset + it->length > next->offset)
    {
        throw std::logic_error("Invalid adjacent extents found! Extent with offset " + std::to_string(prev->offset) + " and length " +
                               std::to_string(prev->length) + " tresspassing into next extent");
    }
}

void ReadLeveledStorage::printExtents() const
{
    for(const auto& extent : extents)
    {
        std::cout << "Offset: " << extent.offset << ", length: " << extent.length << ", merge level: " << extent.merge_level << std::endl;
    }
}

void ReadLeveledStorage::printStats() const
{
    std::cout << "Primary reads: " << stats.primary_reads << ", secondary reads: " << stats.secondary_reads << std::endl;
}