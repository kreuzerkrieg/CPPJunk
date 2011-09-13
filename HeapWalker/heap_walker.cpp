#include "StdAfx.h"
#include "heap_walker.h"
#include "storage_classes.h"

heap_walker_t::heap_walker_t (
	void
	):
m_mem_data(),
	m_mbi_size(sizeof(MEMORY_BASIC_INFORMATION))
{
}

heap_walker_t::~heap_walker_t (
	void
	)
{
}

BOOL heap_walker_t::memory_query_helper (
	HANDLE hProcess,
	LPCVOID mem_address,
	VMQUERY_HELP *pVMQHelp) const
{
	SecureZeroMemory (pVMQHelp, sizeof(*pVMQHelp));

	// Get address of region containing passed memory address.
	MEMORY_BASIC_INFORMATION mbi;
	BOOL success = (VirtualQueryEx (hProcess, mem_address, &mbi, m_mbi_size) == m_mbi_size);

	if (!success)
		return (success); // Bad memory address, return failure

	// Walk starting at the region's base address (which never changes)
	PVOID pvRgnBaseAddress = mbi.AllocationBase;

	// Walk starting at the first block in the region (changes in the loop)
	PVOID pvAddressBlk = pvRgnBaseAddress;

	// Save the memory type of the physical storage block.
	pVMQHelp->dwRgnStorage = mbi.Type;

	for (;;)
	{
		// Get info about the current block.
		success = (VirtualQueryEx (hProcess, pvAddressBlk, &mbi, m_mbi_size) == m_mbi_size);
		if (!success)
			break; // Couldn't get the information; end loop.

		// Is this block in the same region?
		if (mbi.AllocationBase != pvRgnBaseAddress)
			break; // Found a block in the next region; end loop.

		// We have a block contained in the region.

		pVMQHelp->dwRgnBlocks++; // Add another block to the region
		pVMQHelp->RgnSize += mbi.RegionSize; // Add block's size to region size

		// If block has PAGE_GUARD attribute, add 1 to this counter
		if ((mbi.Protect & PAGE_GUARD) == PAGE_GUARD)
			pVMQHelp->dwRgnGuardBlks++;

		// Take a best guess as to the type of physical storage committed to the
		// block. This is a guess because some blocks can convert from MEM_IMAGE
		// to MEM_PRIVATE or from MEM_MAPPED to MEM_PRIVATE; MEM_PRIVATE can
		// always be overridden by MEM_IMAGE or MEM_MAPPED.
		if (pVMQHelp->dwRgnStorage == MEM_PRIVATE)
			pVMQHelp->dwRgnStorage = mbi.Type;

		// Get the address of the next block.
		pvAddressBlk = (PVOID) ((PBYTE) pvAddressBlk + mbi.RegionSize);
	}

	// After examining the region, check to see whether it is a thread stack
	// Windows Vista: Assume stack if region has at least 1 PAGE_GUARD block
	pVMQHelp->bRgnIsAStack = (pVMQHelp->dwRgnGuardBlks > 0);

	return (TRUE);
}

BOOL heap_walker_t::memory_query (
	HANDLE hProcess,
	LPCVOID mem_address,
	memory_region &region) const
{
	// Get the MEMORY_BASIC_INFORMATION for the passed address.
	MEMORY_BASIC_INFORMATION mbi;
	BOOL success = (VirtualQueryEx (hProcess, mem_address, &mbi, m_mbi_size) == m_mbi_size);

	if (!success)
		return (success); // Bad memory address; return failure

	// The MEMORY_BASIC_INFORMATION structure contains valid information.
	// Time to start setting the members of our own VMQUERY structure.

	// First, fill in the block members. We'll fill the region members later.
	memory_block block;
	fill_block_data(block, mbi);

	// Now fill in the region data members.
	fill_region_data(hProcess, mem_address, region, mbi);

	return (success);
}

void heap_walker_t::fill_block_data(
	memory_block &block,
	const MEMORY_BASIC_INFORMATION &mbi) const
{
	switch (mbi.State)
	{
	case MEM_FREE: // Free block (not reserved)
		block.set_base_address(NULL);
		block.set_size(0);
		block.set_protection(0);
		block.set_type(MEM_FREE);
		break;
	case MEM_RESERVE: // Reserved block without committed storage in it.
		block.set_base_address((UINT64) mbi.BaseAddress);
		block.set_size(mbi.RegionSize);
		// For an uncommitted block, mbi.Protect is invalid. So we will 
		// show that the reserved block inherits the protection attribute 
		// of the region in which it is contained.
		block.set_protection(mbi.AllocationProtect);
		block.set_type(MEM_RESERVE);
		break;
	case MEM_COMMIT: // Reserved block with committed storage in it.
		block.set_base_address((UINT64)mbi.BaseAddress);
		block.set_size(mbi.RegionSize);
		block.set_protection(mbi.Protect);
		block.set_type(mbi.Type);
		break;
	default:
		break;
	}
}

void heap_walker_t::fill_region_data(
	HANDLE hProcess,
	LPCVOID mem_address,
	memory_region &region,
	const MEMORY_BASIC_INFORMATION &mbi) const
{
	VMQUERY_HELP VMQHelp;
	switch (mbi.State)
	{
	case MEM_FREE: // Free block (not reserved)
		region.set_base_address ((UINT64)mbi.BaseAddress);
		region.set_protection (mbi.AllocationProtect);
		region.set_size (mbi.RegionSize);
		region.set_type (MEM_FREE);
		region.set_blocks(0);
		region.set_guard_blocks(0);
		region.set_stack(FALSE);
		break;

	case MEM_RESERVE: // Reserved block without committed storage in it.
		region.set_base_address( (UINT64)mbi.AllocationBase);
		region.set_protection ( mbi.AllocationProtect);

		// Iterate through all blocks to get complete region information.         
		memory_query_helper (hProcess, mem_address, &VMQHelp);

		region.set_size ( VMQHelp.RgnSize);
		region.set_type ( VMQHelp.dwRgnStorage);
		region.set_blocks (VMQHelp.dwRgnBlocks);
		region.set_guard_blocks ( VMQHelp.dwRgnGuardBlks);
		region.set_stack (VMQHelp.bRgnIsAStack);
		break;

	case MEM_COMMIT: // Reserved block with committed storage in it.
		region.set_base_address( (UINT64)mbi.AllocationBase);
		region.set_protection ( mbi.AllocationProtect);

		// Iterate through all blocks to get complete region information.         
		memory_query_helper (hProcess, mem_address, &VMQHelp);

		region.set_size(VMQHelp.RgnSize);
		region.set_type( VMQHelp.dwRgnStorage);
		region.set_blocks ( VMQHelp.dwRgnBlocks);
		region.set_guard_blocks( VMQHelp.dwRgnGuardBlks);
		region.set_stack ( VMQHelp.bRgnIsAStack);
		break;

	default:
		break;
	}
}
void heap_walker_t::gather_mem_info (
	DWORD process_id)
{
	HANDLE hProcess = OpenProcess (PROCESS_QUERY_INFORMATION, FALSE, process_id);
	if (hProcess == NULL)
	{
		return;
	}
	m_mem_data.resize (0);
	m_mem_data.reserve (1000);
	// Walk the virtual address space, adding entries to the list box.
	BOOL success = TRUE;
	PVOID mem_address = NULL;

	while (success)
	{
		memory_region mem_region;
		success = memory_query (hProcess, mem_address, mem_region);

		if (success)
		{
			m_mem_data.push_back (mem_region);
			//for (DWORD dwBlock = 0; success && (dwBlock < vmq.dwRgnBlocks); dwBlock++)
			//{
			//	// Get the address of the next region to test.
			//	mem_address = ((PBYTE) mem_address + vmq.BlkSize);
			//	if (dwBlock < vmq.dwRgnBlocks - 1)
			//	{
			//		// Don't query the memory info after the last block.
			//		success = memory_query (hProcess, mem_address, &vmq);
			//		if (success)
			//		{
			//			m_mem_data.push_back (vmq);
			//		}
			//	}
			//}
		}

		// Get the address of the next region to test.
		mem_address = (PVOID)(mem_region.get_base_address() + mem_region.get_size());
	}
	CloseHandle (hProcess);
}
void heap_walker_t::dump_mem_data(
	DWORD process_id,
	const std::wstring file_name
	)
{
	gather_mem_info(process_id);
	std::ofstream stream(file_name, std::ios_base::binary );
	std::vector<memory_region>::const_iterator it_beg = m_mem_data.begin();
	std::vector<memory_region>::const_iterator it_end = m_mem_data.end();
	for (; it_beg != it_end; ++it_beg)
	{
		memory_region tmp = *it_beg;
		size_t struct_size = sizeof(tmp);
		const char *dummy = reinterpret_cast<const char *>(&tmp);
		stream.write(dummy, struct_size); 
	}
	stream.close();
}