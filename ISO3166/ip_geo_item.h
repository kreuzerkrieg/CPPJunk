#pragma once


class ip_geo_item;
namespace boost
{ 
	namespace serialization 
	{
		template<class Archive> inline void save_construct_data(
			Archive & ar, 
			const ip_geo_item * t, 
			const unsigned int file_version
			);
	}
}

class geo_item;

class ISO3166_API ip_geo_item
{
private:
	// serialization friends
	friend class boost::serialization::access;
	template<class Archive> friend void boost::serialization::save_construct_data(
		Archive & ar, 
		const ip_geo_item * t, 
		const unsigned int file_version
		);
	template<class Archive> void serialize(Archive &ar, const unsigned int file_version)
	{
		ar & m_start_ip
			& m_end_ip
			& m_connection_type
			& m_isp_name;
	}
public:
	ip_geo_item(
		const geo_item &g_item
		);
	ip_geo_item(
		unsigned int hash
		);
	virtual ~ip_geo_item(
		void
		);
	void set_start_ip(
		const wstring &ip
		);
	void set_end_ip(
		const wstring &ip
		);
	void set_start_ip(
		const string &ip
		);
	void set_end_ip(
		const string &ip
		);
public:
	typedef enum connection_type
	{
		dialup,
		mobile,
		wireless,
		cable,
		dsl,
		xdsl,
		broadband,
		t1,
		t3,
		oc3,
		oc12,
		satellite,
		connection_type_size
	};
private:
	unsigned long ip_to_long(
		const wstring &ip
		) const;
public:
	unsigned long	m_start_ip;
	unsigned long	m_end_ip;
	connection_type	m_connection_type;
	string			m_isp_name;
	const geo_item	&m_geo_item;
};

namespace boost
{
	namespace serialization
	{
		template<class Archive> inline void save_construct_data(
			Archive & ar,
			const ip_geo_item * t,
			const unsigned int file_version
			)
		{
			unsigned int g_item = t->m_geo_item.get_hash();
			// save data required to construct instance
			ar << g_item;
		}

		template<class Archive> inline void load_construct_data(
			Archive & ar, 
			ip_geo_item * t, 
			const unsigned int file_version
			)
		{
			// retrieve data from archive required to construct new instance
			unsigned int g_item;
			ar >> g_item;
			// invoke inplace constructor to initialize instance of my_class
			::new(t)ip_geo_item(g_item);
		}
	}
}
