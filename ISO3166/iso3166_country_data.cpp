#include "StdAfx.h"
#include "iso3166_country_data.h"

iso3166_country_data *iso3166_country_data::m_instance = NULL;
critical_section iso3166_country_data::m_cs;

iso3166_country_data::iso3166_country_data(
	void
	):
m_empty_country(make_tuple(L"", L"", L"", L"", 0)),
	m_numeric_map(),
	m_name_map(),
	m_alpha2_map(),
	m_alpha3_map(),
	m_ccTLD_map(),
	m_country_map(),
	m_geo_items_map()
{
	FILE *stream = NULL;
	wchar_t line[1024];

	errno_t err = _wfopen_s(&stream, L"c:\\1\\Dev\\ISO3166\\ISO-3166.csv", L"r, ccs=UNICODE");
	if (NULL != stream)
	{
		while (fgetws (line, 1024, stream) != NULL)
		{
			wstring tmp_str (line, wcslen (line));

			vector<wstring> values;
			algorithm::split(values, line, algorithm::is_any_of(L","));
			if (!values.empty())
			{
				assert(values.size()==5);
				//4,AF,AFG,.af,Afghanistan
				unsigned short numeric_country = boost::lexical_cast<unsigned short>(values[0]);
				iso3166_entry tmp_val = make_tuple(
					boost::algorithm::to_upper_copy(values[4]), 
					boost::algorithm::to_upper_copy(values[2]), 
					boost::algorithm::to_upper_copy(values[1]), 
					boost::algorithm::to_upper_copy(values[3]), 
					numeric_country);
				pair<unsigned short, iso3166_entry> pair2add = pair<unsigned short, iso3166_entry>(numeric_country, tmp_val);
				pair <map <unsigned short, iso3166_entry>::iterator, bool> added_pair = m_numeric_map.insert(pair2add);
				m_name_map.insert(pair<wstring&, iso3166_entry&>(get<0>((*(added_pair.first)).second), (*(added_pair.first)).second));
				m_alpha2_map.insert(pair<wstring&, iso3166_entry&>(get<2>((*(added_pair.first)).second), (*(added_pair.first)).second));
				m_alpha3_map.insert(pair<wstring&, iso3166_entry&>(get<1>((*(added_pair.first)).second), (*(added_pair.first)).second));
				m_ccTLD_map.insert(pair<wstring&, iso3166_entry&>(get<3>((*(added_pair.first)).second), (*(added_pair.first)).second));
				m_country_map.insert(pair<wstring&, iso3166_country>(get<0>((*(added_pair.first)).second), iso3166_country((*(added_pair.first)).second)));
			}
		}
	}
	fclose(stream);
}

iso3166_country_data& iso3166_country_data::instance()
{
	m_cs.lock();
	if (NULL == m_instance)
	{
		m_instance = new iso3166_country_data();
	}
	m_cs.unlock();
	return *m_instance;
}

iso3166_country_data::~iso3166_country_data(void)
{
	cout << m_geo_items_map.size() << " entries in geo map." << endl;
	if (NULL != m_instance)
	{
		delete m_instance;
		m_instance = NULL;
	}
}

const iso3166_country_data::iso3166_entry& iso3166_country_data::get_country_entry(
	const unsigned short country_id
	) const
{
	map <unsigned short, iso3166_entry>::const_iterator country = m_numeric_map.find(country_id);
	assert (country != m_numeric_map.end());
	return country->second;
}

const iso3166_country_data::iso3166_entry& iso3166_country_data::get_country_entry(
	const wstring &country_id
	) const
{
	map <wstring, iso3166_entry&>::const_iterator country = m_name_map.find(country_id);
	wstring upper_country = boost::algorithm::to_upper_copy(country_id);
	if (country == m_name_map.end())
	{
		country = m_alpha2_map.find(upper_country);
		if (country == m_alpha2_map.end())
		{
			country = m_alpha3_map.find(upper_country);
			if (country == m_alpha3_map.end())
			{
				country = m_ccTLD_map.find(upper_country);
				std::string err("No country found for token '");
				err.append(CW2A(upper_country.c_str()));
				err.append("'");
				throw std::invalid_argument(err);
			}
		}
	}
	return country->second;
}

const iso3166_country& iso3166_country_data::get_country(
	const unsigned short country_id
	) const
{
	return m_country_map.find(get<0>(get_country_entry(country_id)))->second;
}
const iso3166_country& iso3166_country_data::get_country(
	const wstring &country_id
	) const
{
	map<wstring, iso3166_country>::const_iterator it =	m_country_map.find(get<0>(get_country_entry(country_id)));
	if (it == m_country_map.end())
	{
		std::string err("Cannot find country '");
				err.append(CW2A (country_id.c_str()));
				err.append("'");
		throw invalid_argument (err);
	}
	return it->second;
}

const geo_item& iso3166_country_data::get_geo_item(
	const geo_item &item
	)
{
	map	<unsigned int, geo_item>::const_iterator iter = m_geo_items_map.find(item.get_hash());
	if (iter == m_geo_items_map.end())
	{
		pair<map<unsigned int, geo_item>::iterator, bool> res_pair = 
			m_geo_items_map.insert(pair<unsigned int, geo_item>(item.get_hash(), item));
		if (m_geo_items_map.size() % 10000 == 0)
						cout << m_geo_items_map.size() << " entries in geo map." << endl;
		return res_pair.first->second;
	}
	else
		return iter->second;
}

const geo_item& iso3166_country_data::get_geo_item_by_hash(
	const unsigned int item
	) const
{
	map	<unsigned int, geo_item>::const_iterator iter = m_geo_items_map.find(item);
	if (iter == m_geo_items_map.end())
	{
		throw std::exception("geo item not found");
	}
	else
		return iter->second;
}

const iso3166_country_data::iso3166_entry& iso3166_country_data::get_empty_country(
	) const
{
	return m_empty_country;
}

map <unsigned int, geo_item>& iso3166_country_data::get_country_map(
	)
{
	return m_geo_items_map;
}