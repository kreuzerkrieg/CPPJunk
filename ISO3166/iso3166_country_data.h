#pragma once
#include "critical_section.h"
#include "iso3166_country.h"
#include "geo_item.h"
class ISO3166_API iso3166_country_data
{
private:
	iso3166_country_data(
		void
		);
public:
	typedef tuple<wstring, wstring, wstring, wstring, unsigned short> iso3166_entry;
public:
	static iso3166_country_data& instance();
	virtual ~iso3166_country_data(void);

	const iso3166_country_data::iso3166_entry& get_country_entry(
		const unsigned short country_id
		) const;
	const iso3166_country_data::iso3166_entry& get_country_entry(
		const wstring &country_id
		) const;
	const iso3166_country& get_country(
		const unsigned short country_id
		) const;
	const iso3166_country& get_country(
		const wstring &country_id
		) const;
	const iso3166_country_data::iso3166_entry& get_empty_country(
		) const;
	const geo_item& get_geo_item(
		const geo_item &item
		);
	const geo_item& get_geo_item_by_hash(
		const unsigned int item
		) const;
	map <unsigned int, geo_item>& get_country_map(
		);
private:
	static iso3166_country_data					*m_instance;
	static critical_section						m_cs;

	const iso3166_entry							m_empty_country;
	map <unsigned short, iso3166_entry>			m_numeric_map;
	map	<wstring, iso3166_entry&>				m_name_map;
	map	<wstring, iso3166_entry&>				m_alpha2_map;
	map	<wstring, iso3166_entry&>				m_alpha3_map;
	map	<wstring, iso3166_entry&>				m_ccTLD_map;
	map	<wstring, iso3166_country>				m_country_map;
	map	<unsigned int, geo_item>				m_geo_items_map;
};

