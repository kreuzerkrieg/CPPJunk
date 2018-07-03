#pragma once

#include <cstdint>
#include <bits/types/struct_iovec.h>
#include <future>

using Baddr = uint64_t;
using byte = uint8_t;

class NVMeManager;

class NVMeStriper
{
public:
    NVMeStriper(const std::string& deviceName, NVMeManager& nvmeManager);

    virtual ~NVMeStriper() = default;

    virtual std::future<void> Read(Baddr offset, byte* buffer, uint32_t nbytes);

    virtual std::future<void> Write(Baddr offset, const byte* buffer, uint32_t nbytes);

    virtual std::future<void> ReadV(Baddr offset, iovec* iovec, int niovec);

    virtual std::future<void> WriteV(Baddr offset, iovec* iovec, int niovec);

//    virtual void
//    MultipleIoV(Baddr baddrs[], uint32_t baddrsCount, iovec iovecs[], int niovecs[], bool isWrite);

    virtual uint64_t GetSize() const;

    virtual unsigned int GetBlockSize() const;

    virtual uint64_t GetOptIoSize() const;

    virtual bool IsTrimmed(Baddr offset) const;

    virtual void Trim(Baddr addr, uint64_t baddrsCount);

private:
    NVMeManager& nvmeManager;
    std::string deviceName;
};
