#include "StdAfx.h"
#include "geo_item.h"
#include "hash_utils.h"
#include "iso3166_country_data.h"

geo_item::geo_item(
	unsigned short country_code
	):
m_country(iso3166_country_data::instance().get_country(country_code))
{
}

geo_item::geo_item(
	const wstring &country_name
	):
m_country(iso3166_country_data::instance().get_country(country_name))
{
}

geo_item::~geo_item(void)
{
}

unsigned int geo_item::get_hash(
	) const
{
	unsigned int ret_val = 0;
	ret_val = get_token_fingerprint(
		boost::algorithm::to_upper_copy(m_region),
		ret_val);
	ret_val = get_token_fingerprint(
		boost::algorithm::to_upper_copy(m_city),
		ret_val);
	ret_val = get_token_fingerprint(
		boost::algorithm::to_upper_copy(m_zip_code_text),
		ret_val);
	ret_val = get_token_fingerprint(
		m_country.get_alpha_3(),
		ret_val);
	ret_val = get_token_fingerprint(
		boost::lexical_cast<string>(m_country_conf),
		ret_val);
	ret_val = get_token_fingerprint(
		boost::lexical_cast<string>(m_region_conf),
		ret_val);
	ret_val = get_token_fingerprint(
		boost::lexical_cast<string>(m_city_conf),
		ret_val);
	ret_val = get_token_fingerprint(
		boost::lexical_cast<string>(m_metro_code),
		ret_val);
	ret_val = get_token_fingerprint(
		boost::lexical_cast<string>(m_latitude),
		ret_val);
	ret_val = get_token_fingerprint(
		boost::lexical_cast<string>(m_longitude),
		ret_val);
	ret_val = get_token_fingerprint(
		boost::lexical_cast<string>(m_region_code),
		ret_val);
	ret_val = get_token_fingerprint(
		boost::lexical_cast<string>(m_city_code),
		ret_val);
	ret_val = get_token_fingerprint(
		boost::lexical_cast<string>(m_continent_code),
		ret_val);
	ret_val = get_token_fingerprint(
		boost::lexical_cast<string>(m_area_code),
		ret_val);
	ret_val = get_token_fingerprint(
		boost::lexical_cast<string>(m_zip_code),
		ret_val);
	ret_val = get_token_fingerprint(
		boost::lexical_cast<string>(m_gmt_offset),
		ret_val);
	ret_val = get_token_fingerprint(
		boost::lexical_cast<string>(m_in_dst),
		ret_val);
	return ret_val;
}