#pragma once
class memory_region
{
public:
	memory_region();
	virtual ~memory_region();
	inline unsigned __int64 get_base_address(
		) const
	{
		return m_region_base_addr;
	};
	inline void set_base_address(
		const unsigned __int64 base_address
		)
	{
		m_region_base_addr = base_address;
	};

	inline DWORD get_protection(
		) const
	{
		return m_region_protection;
	};
	inline void set_protection(
		const DWORD protection
		)
	{
		m_region_protection = protection;
	};

	inline unsigned __int64 get_size(
		) const
	{
		return m_size;
	};
	inline void set_size(
		const unsigned __int64 size
		)
	{
		m_size = size;
	};

	inline DWORD get_type(
		) const
	{
		return m_storage_type;
	};
	inline void set_type(
		const DWORD type
		)
	{
		m_storage_type = type;
	};

	inline DWORD get_blocks(
		) const
	{
		return m_region_blocks;
	};
	inline void set_blocks(
		const DWORD blocks
		)
	{
		m_region_blocks = blocks;
	};

	inline DWORD get_guard_blocks(
		) const
	{
		return m_guard_blocks;
	};
	inline void set_guard_blocks(
		const DWORD guard_blocks
		)
	{
		m_guard_blocks = guard_blocks;
	};

	inline BOOL get_stack(
		) const
	{
		return m_is_stack;
	};
	inline void set_stack(
		const BOOL is_stack
		)
	{
		m_is_stack = is_stack;
	};

	inline const std::vector <memory_region>& get_regions(
		) const
	{
		return m_regions;
	};
	inline void add_region(
		const memory_region &region
		)
	{
		m_regions.push_back(region);
	};
private:
	unsigned __int64				m_region_base_addr;
	DWORD							m_region_protection; // PAGE_*
	unsigned __int64				m_size;
	DWORD							m_storage_type; // MEM_*: Free, Image, Mapped, Private
	DWORD							m_region_blocks;
	DWORD							m_guard_blocks; // If > 0, region contains thread stack
	BOOL							m_is_stack;

	std::vector <memory_region>		m_regions;
};

class memory_block
{
public:
	memory_block();
	virtual ~memory_block();
public:
	inline unsigned __int64 get_base_address(
		) const
	{
		return m_block_base_addr;
	};
	inline void set_base_address(
		const unsigned __int64 base_address
		)
	{
		m_block_base_addr = base_address;
	};

	inline DWORD get_protection(
		) const
	{
		return m_block_protection;
	};
	inline void set_protection(
		const DWORD protection
		)
	{
		m_block_protection = protection;
	};

	inline unsigned __int64 get_size(
		) const
	{
		return m_size;
	};
	inline void set_size(
		const unsigned __int64 size
		)
	{
		m_size = size;
	};

	inline DWORD get_type(
		) const
	{
		return m_storage_type;
	};
	inline void set_type(
		const DWORD type
		)
	{
		m_storage_type = type;
	};
private:
	unsigned __int64				m_block_base_addr;
	DWORD							m_block_protection; // PAGE_*
	unsigned __int64				m_size;
	DWORD							m_storage_type; // MEM_*: Free, Reserve, Image, Mapped, Private
};