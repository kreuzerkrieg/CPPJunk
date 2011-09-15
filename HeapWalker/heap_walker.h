#pragma once

class memory_region;
class memory_block;
struct vm_query_helper;

class heap_walker_t
{
public:
	heap_walker_t (
		void);
	virtual ~heap_walker_t (
		void);
public:
	void dump_mem_data(
		DWORD process_id,
		const std::wstring file_name
		);
	
private:
	BOOL memory_query (
		HANDLE hProcess,
		LPCVOID mem_address,
		memory_region &region
		) const;
	BOOL memory_query_helper (
		HANDLE hProcess,
		LPCVOID mem_address,
		memory_region &region,
		vm_query_helper &mem_helper
		) const;
	void fill_block_data(
		memory_block &block,
		const MEMORY_BASIC_INFORMATION &mbi
		) const;
	void fill_region_data(
		HANDLE hProcess,
		LPCVOID mem_address,
		memory_region &region,
		const MEMORY_BASIC_INFORMATION &mbi
		) const;
	void gather_mem_info(
		DWORD process_id
		);
private:
	std::vector<memory_region>	m_mem_data;
	const size_t				m_mbi_size;
};

