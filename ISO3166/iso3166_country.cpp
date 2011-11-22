#include "stdafx.h"
#include "iso3166_country.h"
#include "iso3166_country_data.h"

iso3166_country::iso3166_country(
	void
	):
m_entry(iso3166_country_data::instance().get_empty_country()),
	m_short_name(get<0>(m_entry)),
	m_alpha_3(get<1>(m_entry)),
	m_ccTLD(get<3>(m_entry)),
	m_alpha_2(get<2>(m_entry)),
	m_numeric(get<4>(m_entry))
{
}

iso3166_country::iso3166_country(
	const wstring &country_id
	):
m_entry(iso3166_country_data::instance().get_country_entry(country_id)),
	m_short_name(get<0>(m_entry)),
	m_alpha_3(get<1>(m_entry)),
	m_ccTLD(get<3>(m_entry)),
	m_alpha_2(get<2>(m_entry)),
	m_numeric(get<4>(m_entry))
{
}

iso3166_country::iso3166_country(
	const unsigned short &country_id
	):
m_entry(iso3166_country_data::instance().get_country_entry(country_id)),
	m_short_name(get<0>(m_entry)),
	m_alpha_3(get<1>(m_entry)),
	m_ccTLD(get<3>(m_entry)),
	m_alpha_2(get<2>(m_entry)),
	m_numeric(get<4>(m_entry))
{
}

iso3166_country::iso3166_country(
	const iso3166_entry &country_entry
	):
m_entry(country_entry),
	m_short_name(get<0>(m_entry)),
	m_alpha_3(get<1>(m_entry)),
	m_ccTLD(get<3>(m_entry)),
	m_alpha_2(get<2>(m_entry)),
	m_numeric(get<4>(m_entry))
{
}

iso3166_country::~iso3166_country(void)
{
}

const wstring& iso3166_country::get_alpha_3(
		) const
{
	return m_alpha_3;
}

const unsigned short iso3166_country::get_numeric(
		) const
{
	return m_numeric;
}
