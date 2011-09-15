#include "StdAfx.h"
#include "storage_classes.h"

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
memory_block::memory_block(
	):
m_block_base_addr(0),
	m_block_protection(0),
	m_size(0),
	m_storage_type(0)
{
}
memory_block::~memory_block(
	)
{
}
