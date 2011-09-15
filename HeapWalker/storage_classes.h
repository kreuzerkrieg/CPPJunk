#pragma once

class memory_block;

class memory_region
{
	// serialization friends
	friend class boost::serialization::access;

public:
	memory_region();
	virtual ~memory_region();

	template<class Archive> void serialize(
		Archive & ar, 
		const unsigned int version
		)      
	{
		ar & BOOST_SERIALIZATION_NVP(m_region_base_addr)
		& BOOST_SERIALIZATION_NVP(m_region_protection)
		& BOOST_SERIALIZATION_NVP(m_size)
		& BOOST_SERIALIZATION_NVP(m_storage_type)
		& BOOST_SERIALIZATION_NVP(m_region_blocks)
		& BOOST_SERIALIZATION_NVP(m_guard_blocks)
		& BOOST_SERIALIZATION_NVP(m_is_stack)
		& BOOST_SERIALIZATION_NVP(m_blocks);
	}

public:
	inline UINT64 get_base_address(
		) const
	{
		return m_region_base_addr;
	};
	inline void set_base_address(
		const UINT64 base_address
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

	inline UINT64 get_size(
		) const
	{
		return m_size;
	};
	inline void set_size(
		const UINT64 size
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

	inline const std::vector <memory_block>& get_regions(
		) const
	{
		return m_blocks;
	};
	inline void add_block(
		const memory_block &block
		)
	{
		m_blocks.push_back(block);
	};
private:
	UINT64						m_region_base_addr;
	DWORD						m_region_protection;
	UINT64						m_size;
	DWORD						m_storage_type;
	DWORD						m_region_blocks;
	DWORD						m_guard_blocks;
	BOOL						m_is_stack;
	std::vector <memory_block>	m_blocks;
};

class memory_block
{
	// serialization friends
	friend class boost::serialization::access;
public:
	memory_block();
	virtual ~memory_block();

	template<class Archive> void serialize(
		Archive & ar, 
		const unsigned int version
		)      
	{
		ar & BOOST_SERIALIZATION_NVP(m_block_base_addr)
		& BOOST_SERIALIZATION_NVP(m_block_protection)
		& BOOST_SERIALIZATION_NVP(m_size)
		& BOOST_SERIALIZATION_NVP(m_storage_type);
	}

public:
	inline UINT64 get_base_address(
		) const
	{
		return m_block_base_addr;
	};
	inline void set_base_address(
		const UINT64 base_address
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

	inline UINT64 get_size(
		) const
	{
		return m_size;
	};
	inline void set_size(
		const UINT64 size
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
	UINT64	m_block_base_addr;
	DWORD	m_block_protection;
	UINT64	m_size;
	DWORD	m_storage_type;
};

struct vm_query_helper
{
	vm_query_helper(
		):
	m_region_size(0),
		m_storage_type(0),
		m_region_blocks(0),
		m_guard_blocks(0),
		m_is_stack(FALSE)
	{
	}
	~vm_query_helper(
		)
	{
	}
	UINT64	m_region_size;
	DWORD	m_storage_type;
	DWORD	m_region_blocks;
	DWORD	m_guard_blocks;
	BOOL	m_is_stack;
};