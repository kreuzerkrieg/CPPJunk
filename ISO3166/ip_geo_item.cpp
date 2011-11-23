#include "StdAfx.h"
#include "ip_geo_item.h"
#include "iso3166_country_data.h"

ip_geo_item::ip_geo_item(
	const geo_item &g_item
	):
m_geo_item((iso3166_country_data::instance().get_geo_item(g_item)))
{
}

ip_geo_item::ip_geo_item(
		unsigned int hash
		):
m_geo_item((iso3166_country_data::instance().get_geo_item_by_hash(hash)))
{
}

ip_geo_item::~ip_geo_item(void)
{
}

void ip_geo_item::set_start_ip(
	const wstring &ip
	)
{
	m_start_ip = ip_to_long(ip);
}

void ip_geo_item::set_end_ip(
	const wstring &ip
	)
{
	m_end_ip = ip_to_long(ip);
}

void ip_geo_item::set_start_ip(
	const string &ip
	)
{
	m_start_ip = ip_to_long(wstring(CA2W(ip.c_str())));
}

void ip_geo_item::set_end_ip(
	const string &ip
	)
{
	m_end_ip = ip_to_long(wstring(CA2W(ip.c_str())));
}

unsigned long ip_geo_item::ip_to_long(
	const wstring &ip
	)const
{
	unsigned long ret_val = 0;
	vector<wstring> octets;
	algorithm::split(octets, ip, algorithm::is_any_of(L"."));
	assert (octets.size()==4);
	for (int i = 0; i < 4; i++)
	{
		ret_val <<= 8;
		ret_val |= (unsigned char)(boost::lexical_cast<int>(octets[i]));
	}
	return ret_val;
}
