#pragma once

class memory_region;
class memory_block;

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
	//typedef struct
	//{
	//	// Region information
	//	unsigned __int64 pvRgnBaseAddress;
	//	DWORD dwRgnProtection; // PAGE_*
	//	unsigned __int64 RgnSize;
	//	DWORD dwRgnStorage; // MEM_*: Free, Image, Mapped, Private
	//	DWORD dwRgnBlocks;
	//	DWORD dwRgnGuardBlks; // If > 0, region contains thread stack
	//	BOOL bRgnIsAStack; // TRUE if region contains thread stack

	//	// Block information
	//	unsigned __int64 pvBlkBaseAddress;
	//	DWORD dwBlkProtection; // PAGE_*
	//	unsigned __int64 BlkSize;
	//	DWORD dwBlkStorage; // MEM_*: Free, Reserve, Image, Mapped, Private
	//} VMQUERY, *PVMQUERY;

	// Helper structure
	typedef struct
	{
		SIZE_T RgnSize;
		DWORD dwRgnStorage; // MEM_*: Free, Image, Mapped, Private
		DWORD dwRgnBlocks;
		DWORD dwRgnGuardBlks; // If > 0, region contains thread stack
		BOOL bRgnIsAStack; // TRUE if region contains thread stack
	} VMQUERY_HELP;
private:
	BOOL memory_query (
		HANDLE hProcess,
		LPCVOID mem_address,
		memory_region &region
		) const;
	BOOL memory_query_helper (
		HANDLE hProcess,
		LPCVOID mem_address,
		VMQUERY_HELP *pVMQHelp
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

