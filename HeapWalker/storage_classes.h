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
		UINT64 region_size = get_size();
		ar & BOOST_SERIALIZATION_NVP(m_region_base_addr)
			& BOOST_SERIALIZATION_NVP(m_region_protection)
			& BOOST_SERIALIZATION_NVP(region_size)
			& BOOST_SERIALIZATION_NVP(m_storage_type)
			& BOOST_SERIALIZATION_NVP(m_region_blocks)
			& BOOST_SERIALIZATION_NVP(m_guard_blocks)
			& BOOST_SERIALIZATION_NVP(m_is_stack)
			& BOOST_SERIALIZATION_NVP(m_blocks);
	}
public:
	UINT64 get_base_address(
		) const;
	void set_base_address(
		const UINT64 base_address
		);

	DWORD get_protection(
		) const;
	void set_protection(
		const DWORD protection
		);

	UINT64 get_size(
		) const;
	void set_size(
		const UINT64 size
		);

	DWORD get_type(
		) const;
	void set_type(
		const DWORD type
		);
	DWORD get_blocks(
		) const;
	void set_blocks(
		const DWORD blocks
		);

	DWORD get_guard_blocks(
		) const;
	void set_guard_blocks(
		const DWORD guard_blocks
		);
	BOOL get_stack(
		) const;
	void set_stack(
		const BOOL is_stack
		);

	const std::vector <memory_block>& get_mem_blocks(
		) const;
	void add_block(
		const memory_block &block
		);
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

struct vm_query_helper
{
	vm_query_helper(
		):
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
	DWORD	m_storage_type;
	DWORD	m_region_blocks;
	DWORD	m_guard_blocks;
	BOOL	m_is_stack;
};