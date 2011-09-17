#include "StdAfx.h"
#include "heap_walker.h"
#include "storage_classes.h"
#include "memory_block.h"

heap_walker_t::heap_walker_t (
	DWORD process_id
	):
m_process(NULL),
	m_process_id(process_id),
	m_mem_data(),
	m_mbi_size(sizeof(MEMORY_BASIC_INFORMATION))
{
	if (process_id == 0)
	{
		process_id = GetCurrentProcessId();
	}

	m_process = OpenProcess (PROCESS_QUERY_INFORMATION, FALSE, process_id);

	if (m_process == NULL)
	{
		std::string ex (
			"Cannot open process '" + 
			boost::lexical_cast<string>(process_id) + 
			"' for querying information."
			);
		DWORD err = GetLastError();
		if (0 != err)
		{
			LPTSTR buff = NULL;
			FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM |
				FORMAT_MESSAGE_ALLOCATE_BUFFER,
				0,
				err,
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
				(LPTSTR)&buff, 
				0, 
				0);
			ex.append("\nReason: ");
			//ex.append(string(T2CA(buff)));
			LocalFree(buff);
		}

		throw std::exception (ex.c_str());
	}
	m_mem_data.reserve (1000);
}

heap_walker_t::~heap_walker_t (
	void
	)
{
	//CloseHandle(m_process);
}

BOOL heap_walker_t::memory_query_helper (
	LPCVOID mem_address,
	memory_region &region,
	vm_query_helper &mem_helper) const
{
	// Get address of region containing passed memory address.
	MEMORY_BASIC_INFORMATION mbi;
	BOOL success = (VirtualQueryEx (m_process, mem_address, &mbi, m_mbi_size) == m_mbi_size);

	if (!success)
		return (success); // Bad memory address, return failure

	// Walk starting at the region's base address (which never changes)
	PVOID pvRgnBaseAddress = mbi.AllocationBase;

	// Walk starting at the first block in the region (changes in the loop)
	PVOID pvAddressBlk = pvRgnBaseAddress;

	// Save the memory type of the physical storage block.
	mem_helper.m_storage_type = mbi.Type;

	for (;;)
	{
		// Get info about the current block.
		success = (VirtualQueryEx (m_process, pvAddressBlk, &mbi, m_mbi_size) == m_mbi_size);
		if (!success)
			break; // Couldn't get the information; end loop.

		// Is this block in the same region?
		if (mbi.AllocationBase != pvRgnBaseAddress)
			break; // Found a block in the next region; end loop.

		// We have a block contained in the region.

		mem_helper.m_region_blocks++; // Add another block to the region

		// If block has PAGE_GUARD attribute, add 1 to this counter
		if ((mbi.Protect & PAGE_GUARD) == PAGE_GUARD)
			mem_helper.m_guard_blocks++;

		// Get the address of the next block.
		pvAddressBlk = (PVOID) ((PBYTE) pvAddressBlk + mbi.RegionSize);

		memory_block block(region);
		fill_block_data(block, mbi);
		region.add_block(block);
	}

	// After examining the region, check to see whether it is a thread stack
	// Windows Vista: Assume stack if region has at least 1 PAGE_GUARD block
	mem_helper.m_is_stack = (mem_helper.m_guard_blocks > 0);

	return (TRUE);
}

BOOL heap_walker_t::memory_query (
	LPCVOID mem_address,
	memory_region &region) const
{
	// Get the MEMORY_BASIC_INFORMATION for the passed address.
	MEMORY_BASIC_INFORMATION mbi;
	BOOL success = (VirtualQueryEx (m_process, mem_address, &mbi, m_mbi_size) == m_mbi_size);

	if (!success)
		return (success); // Bad memory address; return failure

	// Now fill in the region data members.
	fill_region_data(mem_address, region, mbi);

	return (success);
}

//void heap_walker_t::fill_block_data(
//	memory_block &block,
//	const MEMORY_BASIC_INFORMATION &mbi) const
//{
//	switch (mbi.State)
//	{
//	case MEM_FREE: // Free block (not reserved)
//		block.set_base_address(NULL);
//		block.set_size(0);
//		block.set_protection(0);
//		block.set_type(MEM_FREE);
//		break;
//	case MEM_RESERVE: // Reserved block without committed storage in it.
//		block.set_base_address((UINT64) mbi.BaseAddress);
//		block.set_size(mbi.RegionSize);
//		block.set_type(MEM_RESERVE);
//	case MEM_COMMIT:  // Reserved block with committed storage in it.
//		block.set_base_address((UINT64) mbi.BaseAddress);
//		block.set_size(mbi.RegionSize);
//		block.set_protection(mbi.Protect);
//		block.set_type(mbi.Type);
//		break;
//	default:
//		break;
//	}
//	get_additional_info(block);
//}
//
//void heap_walker_t::fill_region_data(
//	LPCVOID mem_address,
//	memory_region &region,
//	const MEMORY_BASIC_INFORMATION &mbi
//	) const
//{
//	vm_query_helper mem_helper;
//	switch (mbi.State)
//	{
//	case MEM_FREE: // Free block (not reserved)
//		region.set_base_address ((UINT64)mbi.BaseAddress);
//		region.set_protection (mbi.AllocationProtect);
//		region.set_size (mbi.RegionSize);
//		region.set_type (MEM_FREE);
//		region.set_blocks(0);
//		region.set_guard_blocks(0);
//		region.set_stack(FALSE);
//		break;
//
//	case MEM_RESERVE:
//	case MEM_COMMIT:
//		region.set_base_address( (UINT64)mbi.AllocationBase);
//		region.set_protection ( mbi.AllocationProtect);
//		region.set_size ( mbi.RegionSize);
//
//		// Iterate through all blocks to get complete region information.         
//		memory_query_helper (mem_address, region, mem_helper);
//		assert(mem_helper.m_storage_type == mbi.Type);
//		region.set_type ( mem_helper.m_storage_type);
//		region.set_blocks (mem_helper.m_region_blocks);
//		region.set_guard_blocks ( mem_helper.m_guard_blocks);
//		region.set_stack (mem_helper.m_is_stack);
//		break;
//
//	default:
//		assert(false);
//	}
//}
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
	LPCVOID mem_address,
	memory_region &region,
	const MEMORY_BASIC_INFORMATION &mbi) const
{
	vm_query_helper mem_helper;
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
		memory_query_helper (mem_address, region, mem_helper);

		//region.set_size ( mem_helper.m_region_size);
		region.set_type ( mem_helper.m_storage_type);
		region.set_blocks (mem_helper.m_region_blocks);
		region.set_guard_blocks ( mem_helper.m_guard_blocks);
		region.set_stack (mem_helper.m_is_stack);
		break;

	case MEM_COMMIT: // Reserved block with committed storage in it.
		region.set_base_address( (UINT64)mbi.AllocationBase);
		region.set_protection ( mbi.AllocationProtect);

		// Iterate through all blocks to get complete region information.         
		memory_query_helper (mem_address, region, mem_helper);

		//region.set_size(mem_helper.m_region_size);
		region.set_type( mem_helper.m_storage_type);
		region.set_blocks ( mem_helper.m_region_blocks);
		region.set_guard_blocks( mem_helper.m_guard_blocks);
		region.set_stack ( mem_helper.m_is_stack);
		break;

	default:
		break;
	}
}
void heap_walker_t::get_additional_info(
	memory_block &block
	) const
{
	PSAPI_WORKING_SET_EX_INFORMATION mem_ex_info;
	SecureZeroMemory(&mem_ex_info, sizeof(mem_ex_info));
	mem_ex_info.VirtualAddress = (PVOID)block.get_base_address();
	if (QueryWorkingSetEx(m_process, &mem_ex_info, sizeof(mem_ex_info)))
	{
		block.set_shared(mem_ex_info.VirtualAttributes.Valid && mem_ex_info.VirtualAttributes.Shared);
	}
	if(block.get_type() == MEM_IMAGE ||
		block.get_type() == MEM_MAPPED)
	{
		TCHAR file_name[MAX_PATH+1];
		if (GetMappedFileName(m_process, (PVOID)block.get_base_address(), file_name, MAX_PATH))
		{
			block.set_file_name(wstring(T2CW(file_name)));
		}
		else
		{
			DWORD err = GetLastError();
			if (0 != err)
			{
				LPTSTR buff = NULL;
				FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM |
					FORMAT_MESSAGE_ALLOCATE_BUFFER,
					0,
					err,
					MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
					(LPTSTR)&buff, 
					0, 
					0);
				LocalFree(buff);
			}
		}
	}
}

void heap_walker_t::gather_mem_info (
	)
{
	m_mem_data.resize (0);
	m_mem_data.reserve (1000);
	// Walk the virtual address space, adding entries to the list box.
	BOOL success = TRUE;
	PVOID mem_address = NULL;

	while (success)
	{
		memory_region mem_region;
		success = memory_query (mem_address, mem_region);

		if (success)
		{
			m_mem_data.push_back (mem_region);
		}

		// Get the address of the next region to test.
		mem_address = (PVOID)(mem_region.get_base_address() + mem_region.get_size());
	}
	CloseHandle (m_process);
}
void heap_walker_t::dump_mem_data(
	std::ostream &output_stream
	)
{
	gather_mem_info();
	xml_oarchive archive(output_stream);
	archive << BOOST_SERIALIZATION_NVP(m_mem_data);
}