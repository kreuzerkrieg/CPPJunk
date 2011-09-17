#include "StdAfx.h"
#include "memory_block.h"
#include "storage_classes.h"

memory_block::memory_block(
	const memory_region &parent_region
	):
m_block_base_addr(0),
	m_block_protection(0),
	m_size(0),
	m_storage_type(0),
	m_is_shared(FALSE),
	m_map_file_name(L"\0", MAX_PATH+1),
	m_parent_region(&parent_region)
{
}
memory_block::~memory_block(
	)
{
}

UINT64 memory_block::get_base_address(
	) const
{
	return m_block_base_addr;
}
void memory_block::set_base_address(
	const UINT64 base_address
	)
{
	m_block_base_addr = base_address;
}

DWORD memory_block::get_protection(
	) const
{
	if (m_storage_type == MEM_RESERVE)
	{
		// For an uncommitted block, Protect is invalid. So we will 
		// show that the reserved block inherits the protection attribute 
		// of the region in which it is contained.
		return m_parent_region->get_protection();
	}
	else
	{
		return m_block_protection;
	}
}
void memory_block::set_protection(
	const DWORD protection
	)
{
	m_block_protection = protection;
}

UINT64 memory_block::get_size(
	) const
{
	return m_size;
}
void memory_block::set_size(
	const UINT64 size
	)
{
	m_size = size;
}

DWORD memory_block::get_type(
	) const
{
	return m_storage_type;
}
void memory_block::set_type(
	const DWORD type
	)
{
	m_storage_type = type;
}

BOOL memory_block::get_shared(
	) const
{
	return m_is_shared;
}
void memory_block::set_shared(
	const BOOL shared
	)
{
	m_is_shared = shared;
}

wstring& memory_block::get_file_name(
	)
{
	return m_map_file_name;
}
void memory_block::set_file_name(
	const wstring &file_name
	)
{
	m_map_file_name.assign(file_name);
}