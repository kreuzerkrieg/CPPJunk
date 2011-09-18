#include "StdAfx.h"
#include "storage_classes.h"
#include "memory_block.h"

memory_region::memory_region(
	):
m_region_base_addr(0),
	m_region_protection(0),
	m_size(0),
	m_storage_type(0),
	m_region_blocks(0),
	m_guard_blocks(0),
	m_is_stack(FALSE),
	m_blocks()
{
}

memory_region::~memory_region(
	)
{
}

UINT64 memory_region::get_base_address(
	) const
{
	return m_region_base_addr;
}
void memory_region::set_base_address(
	const UINT64 base_address
	)
{
	m_region_base_addr = base_address;
}

DWORD memory_region::get_protection(
	) const
{
	return m_region_protection;
}
void memory_region::set_protection(
	const DWORD protection
	)
{
	m_region_protection = protection;
}

UINT64 memory_region::get_size(
	) const
{
	if (m_blocks.empty())
	{
		return m_size;
	}
	else
	{
		UINT64 size = 0;
		std::vector<memory_block>::const_iterator it_beg = m_blocks.begin();
		std::vector<memory_block>::const_iterator it_end = m_blocks.end();
		for(; it_beg != it_end; ++it_beg)
		{
			size += it_beg->get_size();
		}
		return size;
	}
}
void memory_region::set_size(
	const UINT64 size
	)
{
	m_size = size;
}

DWORD memory_region::get_type(
	) const
{
	return m_storage_type;
}
void memory_region::set_type(
	const DWORD type
	)
{
	m_storage_type = type;
}

DWORD memory_region::get_blocks(
	) const
{
	return m_region_blocks;
}
void memory_region::set_blocks(
	const DWORD blocks
	)
{
	m_region_blocks = blocks;
}

DWORD memory_region::get_guard_blocks(
	) const
{
	return m_guard_blocks;
}
void memory_region::set_guard_blocks(
	const DWORD guard_blocks
	)
{
	m_guard_blocks = guard_blocks;
}

BOOL memory_region::get_stack(
	) const
{
	return m_is_stack;
}
void memory_region::set_stack(
	const BOOL is_stack
	)
{
	m_is_stack = is_stack;
}

const std::vector <memory_block>& memory_region::get_mem_blocks(
	) const
{
	return m_blocks;
}
void memory_region::add_block(
	const memory_block &block
	)
{
	m_blocks.push_back(block);
}
