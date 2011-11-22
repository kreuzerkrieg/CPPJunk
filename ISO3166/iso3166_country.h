#pragma once

class ISO3166_API iso3166_country
{	
private:
	iso3166_country(
		void
		);
public:
	typedef tuple<wstring, wstring, wstring, wstring, unsigned short> iso3166_entry;
public:
	iso3166_country(
		const wstring &short_name
		);
	iso3166_country(
		const unsigned short &country_id
		);
	iso3166_country(
		const iso3166_entry &country_entry
		);
	virtual ~iso3166_country(
		void
		);
	const wstring& get_alpha_3(
		) const;
	const unsigned short get_numeric(
		) const;
private:
	const iso3166_entry							&m_entry;
	const wstring								&m_short_name;
	const wstring								&m_alpha_3;
	const wstring								&m_ccTLD;
	const wstring								&m_alpha_2;
	const unsigned short						m_numeric;
};

