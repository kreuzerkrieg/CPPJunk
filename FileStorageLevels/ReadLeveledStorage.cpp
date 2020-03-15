#include "Storage.h"
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/serialization/set.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid_serialize.hpp>
#include <chrono>
#include <cstring>
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <unistd.h>

namespace fs = std::filesystem;
using namespace std::chrono_literals;

ReadLeveledStorage::ReadLeveledStorage(const Storage& first_, const Storage& second_)
{
    extfile_name = first_.path;
    extfile_name.replace_extension("extents");

    if(!fs::exists(first_.path) || !loadExtentFile() || signature != boost::lexical_cast<boost::uuids::uuid>(sig_str))
    {
        // TODO: log problem
        std::error_code ec;
        fs::remove(extfile_name, ec);
        fs::remove(first_.path, ec);
        extents.clear();
        extents.emplace(dummy);
        is_fully_primed = false;
        primed_file_size = 0;
        signature = boost::lexical_cast<boost::uuids::uuid>(sig_str);
    }
    file_size = fs::file_size(second_.path);
    first_fd = open(first_.path.c_str(), O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    checkAndThrow(first_fd);
    if(file_size == 0)
    {
        is_fully_primed = true;
    }
    else
    {
        second_fd = open(second_.path.c_str(), O_RDONLY);
        checkAndThrow(second_fd);
        auto res = ftruncate(first_fd, file_size);
        checkAndThrow(res);
        extent_thread = std::thread([this]() {
            while(!shutting_down && !is_fully_primed)
            {
                std::this_thread::sleep_for(1s);
                saveExtentFile();
            }
        });
    }
}

ReadLeveledStorage::~ReadLeveledStorage()
{
    try
    {
        shutting_down = true;
        if(extent_thread.joinable())
        {
            extent_thread.join();
        }
        saveExtentFile();
    }
    catch(...)
    {
        // TODO: log problem
    }
}

ssize_t ReadLeveledStorage::read(void* buff, size_t buff_size)
{
    if(buff_size == 0)
    {
        return 0;
    }
    if(is_fully_primed)
    {
        ++stats.primary_reads;
        return ::read(first_fd, buff, buff_size);
    }

    ssize_t ret_val = 0;
    while(buff_size > 0)
    {
        if(extents.empty())
        {
            throw std::logic_error("Extents list is empty, it should't happen");
        }

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
        else if(extent->offset > position)
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
    if(position == 0 && (*extents.begin()) == dummy)
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
        throw std::logic_error("Current primary file size " + std::to_string(primed_file_size) + " is greater than secondary file size " +
                               std::to_string(file_size));
    }
    if(is_fully_primed)
    {
        extents.clear();
        extent_thread.join();
    }
    return write_res;
}

void ReadLeveledStorage::mergeAdjacent(Extents::const_iterator it)
{
    validateAdjacent(it);
    {
        if(it != extents.begin())
        {
            auto prev = std::prev(it);
            if(prev->offset + prev->length == it->offset)
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
    if(extents.begin() != it)
    {
        auto prev = std::prev(it);
        if(prev->offset + prev->length > it->offset)
        {
            throw std::logic_error("Invalid adjacent extents found! Extent with offset " + std::to_string(prev->offset) + " and length " +
                                   std::to_string(prev->length) + " tresspassing into next extent with offset " +
                                   std::to_string(it->offset) + " and length " + std::to_string(it->length));
        }
    }
    auto next = std::next(it);
    if(next != extents.end() && it->offset + it->length > next->offset)
    {
        throw std::logic_error("Invalid adjacent extents found! Extent with offset " + std::to_string(it->offset) + " and length " +
                               std::to_string(it->length) + " tresspassing into next extent with offset " + std::to_string(next->offset) +
                               " and length " + std::to_string(next->length));
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

void ReadLeveledStorage::saveExtentFile()
{
    std::ofstream ifs(extfile_name, std::ios_base::binary | std::ios_base::trunc);
    boost::archive::binary_oarchive oa(ifs);
    oa << *this;
}

bool ReadLeveledStorage::loadExtentFile()
{
    if(!fs::exists(extfile_name))
    {
        return false;
    }

    try
    {
        std::ifstream ifs(extfile_name, std::ios_base::binary);
        boost::archive::binary_iarchive ia(ifs);
        ia >> (*this);
    }
    catch(std::exception& ex)
    {
        std::cout << ex.what() << std::endl;
        return false;
    }
    return true;
}