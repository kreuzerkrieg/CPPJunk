#pragma once

class memory_region;

class memory_block
{
	// serialization friends
	friend class boost::serialization::access;
public:
	memory_block(
		const memory_region &parent_region
		);
	virtual ~memory_block();

	template<class Archive> void serialize(
	Archive & ar, 
	const unsigned int version
	)
{
	DWORD protection = get_protection();
	ar & BOOST_SERIALIZATION_NVP(m_block_base_addr)
		& BOOST_SERIALIZATION_NVP(protection)
		& BOOST_SERIALIZATION_NVP(m_size)
		& BOOST_SERIALIZATION_NVP(m_storage_type)
		& BOOST_SERIALIZATION_NVP(m_is_shared)
		& BOOST_SERIALIZATION_NVP(m_map_file_name);
}

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

	BOOL get_shared(
		) const;
	void set_shared(
		const BOOL shared
		);

	wstring& get_file_name(
		);
	void set_file_name(
		const wstring &file_name
		);
private:
	UINT64				m_block_base_addr;
	DWORD				m_block_protection;
	UINT64				m_size;
	DWORD				m_storage_type;
	BOOL				m_is_shared;
	wstring				m_map_file_name;
	const memory_region	*m_parent_region;
};