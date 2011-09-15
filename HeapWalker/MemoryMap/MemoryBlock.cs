using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace MemoryMap
{
    public enum PageProtection : uint
    {
        PAGE_NOACCESS = 0x01,
        PAGE_READONLY = 0x02,
        PAGE_READWRITE = 0x04,
        PAGE_WRITECOPY = 0x08,
        PAGE_EXECUTE = 0x10,
        PAGE_EXECUTE_READ = 0x20,
        PAGE_EXECUTE_READWRITE = 0x40,
        PAGE_EXECUTE_WRITECOPY = 0x80,
        PAGE_GUARD = 0x100,
        PAGE_NOCACHE = 0x200,
        PAGE_WRITECOMBINE = 0x400
    };

    public enum MemoryStorageType : uint
    {
        MEM_COMMIT = 0x1000,
        MEM_RESERVE = 0x2000,
        MEM_DECOMMIT = 0x4000,
        MEM_RELEASE = 0x8000,
        MEM_FREE = 0x10000,
        MEM_PRIVATE = 0x20000,
        MEM_MAPPED = 0x40000,
        MEM_RESET = 0x80000,
        MEM_TOP_DOWN = 0x100000,
        MEM_WRITE_WATCH = 0x200000,
        MEM_PHYSICAL = 0x400000,
        MEM_ROTATE = 0x800000,
        SEC_IMAGE = 0x1000000,
        MEM_IMAGE = SEC_IMAGE,
        MEM_LARGE_PAGES = 0x20000000,
        MEM_4MB_PAGES = 0x80000000
    };
    class MemoryRegion
    {
        public UInt64 RegionAddress;
        public UInt32 RegionProtection; // PAGE_*
        public UInt64 RegionSize;
        public UInt32 RegionStorageType; // MEM_*: Free, Image, Mapped, Private
        public UInt32 NumberOfBlocks;
        public UInt32 NumberOfGuardBlocks; // If > 0, region contains thread stack
        public Boolean IsAStack; // TRUE if region contains thread stack

        public List<MemoryBlock> MemoryBlocks;
    };
    class MemoryBlock
    {

        public UInt64 BlockAddress;
        public UInt32 BlockProtection; // PAGE_*
        public UInt64 BlockSize;
        public UInt32 BlockStorageType; // MEM_*: Free,
    }
}
