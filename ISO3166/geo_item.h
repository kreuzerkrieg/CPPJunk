#pragma once
#include "iso3166_country.h"

class geo_item;
namespace boost
{ 
	namespace serialization 
	{
		template<class Archive> inline void save_construct_data(
			Archive & ar, 
			const geo_item * t, 
			const unsigned int file_version
			);

		template<class Archive> inline void load_construct_data(
			Archive & ar, 
			geo_item * t, 
			const unsigned int file_version
			);
	}
}

class ISO3166_API geo_item
{
private:
	// serialization friends
	friend class boost::serialization::access;
	template<class Archive> friend void boost::serialization::save_construct_data(
		Archive & ar, 
		const geo_item * t, 
		const unsigned int file_version
		);

	template<class Archive> friend void boost::serialization::load_construct_data(
		Archive & ar, 
		geo_item * t, 
		const unsigned int file_version
		);
	template<class Archive> void serialize(Archive &ar, const unsigned int file_version)
	{
		ar & BOOST_SERIALIZATION_NVP(m_region)
			& BOOST_SERIALIZATION_NVP(m_city)
			& BOOST_SERIALIZATION_NVP(m_country_conf)
			& BOOST_SERIALIZATION_NVP(m_region_conf)
			& BOOST_SERIALIZATION_NVP(m_city_conf)
			& BOOST_SERIALIZATION_NVP(m_metro_code)
			& BOOST_SERIALIZATION_NVP(m_latitude)
			& BOOST_SERIALIZATION_NVP(m_longitude)
			& BOOST_SERIALIZATION_NVP(m_region_code)
			& BOOST_SERIALIZATION_NVP(m_city_code)
			& BOOST_SERIALIZATION_NVP(m_continent_code)
			& BOOST_SERIALIZATION_NVP(m_area_code)
			& BOOST_SERIALIZATION_NVP(m_zip_code)
			& BOOST_SERIALIZATION_NVP(m_gmt_offset)
			& BOOST_SERIALIZATION_NVP(m_in_dst)
			& BOOST_SERIALIZATION_NVP(m_zip_code_text);
	}
public:
	geo_item(
		unsigned short country_code
		);
	geo_item(
		const wstring &country_name
		);
	virtual ~geo_item(
		void
		);
	unsigned int get_hash(
		) const;
public:
	const iso3166_country	&m_country;
	string					m_region;
	string					m_city;
	unsigned int			m_country_conf;
	unsigned int			m_region_conf;
	unsigned int			m_city_conf;
	unsigned int			m_metro_code;
	double					m_latitude;
	double					m_longitude;
	unsigned int			m_region_code;
	unsigned int			m_city_code;
	unsigned int			m_continent_code;
	unsigned int			m_area_code;
	unsigned int			m_zip_code;
	__time64_t				m_gmt_offset;
	bool					m_in_dst;
	string					m_zip_code_text;
};

namespace boost
{
	namespace serialization
	{
		template<class Archive> inline void save_construct_data(
			Archive & ar,
			const geo_item * t,
			const unsigned int file_version
			)
		{
			unsigned short country_code = t->m_country.get_numeric();
			// save data required to construct instance
			ar << country_code;
		}

		template<class Archive> inline void load_construct_data(
			Archive & ar, 
			geo_item * t, 
			const unsigned int file_version
			)
		{
			// retrieve data from archive required to construct new instance
			unsigned short country_code;
			ar >> country_code;
			// invoke inplace constructor to initialize instance of my_class
			::new(t)geo_item(country_code);
		}
	}
}