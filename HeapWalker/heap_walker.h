#pragma once

class memory_region;
class memory_block;
struct vm_query_helper;

class heap_walker_t
{
public:
	heap_walker_t (
		DWORD process_id
		);
	virtual ~heap_walker_t (
		void);
public:
	void dump_mem_data(
		std::ostream &output_stream
		);
	void dump_csv_mem_data(
		std::ostream &output_stream
		);
private:
	BOOL memory_query (
		LPCVOID mem_address,
		memory_region &region
		) const;
	BOOL memory_query_helper (
		LPCVOID mem_address,
		memory_region &region,
		vm_query_helper &mem_helper
		) const;
	void fill_block_data(
		memory_block &block,
		const MEMORY_BASIC_INFORMATION &mbi
		) const;
	void fill_region_data(
		LPCVOID mem_address,
		memory_region &region,
		const MEMORY_BASIC_INFORMATION &mbi
		) const;
	void get_additional_info(
		memory_block &block
		) const;
	void gather_mem_info(
		);
private:
	HANDLE						m_process;
	DWORD						m_process_id;
	std::vector<memory_region>	m_mem_data;
	const size_t				m_mbi_size;
};

