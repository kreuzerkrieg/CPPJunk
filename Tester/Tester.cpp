// Tester.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <ip_geo_item.h>
#include <geo_item.h>
#include <iso3166_country_data.h>
#include <boost\archive\archive_exception.hpp>
#include <boost\uuid\uuid.hpp>
#include <boost\uuid\uuid_generators.hpp>
#include <boost\random.hpp>
#include <boost/uuid/uuid_io.hpp>
//int _tmain(int argc, _TCHAR* argv[])
//{
//	map <pair<unsigned long, unsigned long>, ip_geo_item> geo_map;
//	FILE *stream = NULL;
//	char line[5*1024];
//	long counter = 0;
//	vector<string> values(28);
//	boost::timer timer;
//	//errno_t err = fopen_s(&stream, "c:\\1\\Dev\\ISO3166\\data.txt", "r");
//	errno_t err = fopen_s(&stream, "c:\\1\\Dev\\ISO3166\\fulldata.txt", "r");
//	if (NULL != stream)
//	{
//		timer.restart();
//		while (fgets (line, 5*1024, stream) != NULL)
//		{
//			values.clear();
//			algorithm::split(values, line, algorithm::is_any_of("|"));
//			if (!values.empty())
//			{
//				assert(values.size()==25);
//				try
//				{
//					geo_item g_item(wstring(CA2W(values[2].c_str())));
//					g_item.m_region=values[3];
//					g_item.m_city=values[4];
//					g_item.m_country_conf=boost::lexical_cast<int>(values[6]);
//					g_item.m_region_conf=boost::lexical_cast<int>(values[7]);
//					g_item.m_city_conf=boost::lexical_cast<int>(values[8]);
//					g_item.m_metro_code=boost::lexical_cast<int>(values[9]);
//					g_item.m_latitude=boost::lexical_cast<double>(values[10]);
//					g_item.m_longitude=boost::lexical_cast<double>(values[11]);
//					g_item.m_region_code=boost::lexical_cast<int>(values[13]);
//					g_item.m_city_code=boost::lexical_cast<int>(values[14]);
//					g_item.m_continent_code=boost::lexical_cast<int>(values[15]);
//					g_item.m_area_code=boost::lexical_cast<int>(values[17]);
//					g_item.m_zip_code=boost::lexical_cast<int>(values[18]);
//					g_item.m_gmt_offset=boost::lexical_cast<int>(values[19]);
//					g_item.m_in_dst=values[20] != "n";
//					g_item.m_zip_code_text=values[21];
//
//					ip_geo_item item(g_item);
//					item.set_start_ip(values[0]);
//					item.set_end_ip(values[1]);
//					item.m_isp_name = values[23];
//					if (values[5] == "broadband")
//						item.m_connection_type = ip_geo_item::broadband;
//					else if (values[5] == "cable")
//						item.m_connection_type = ip_geo_item::cable;
//					else if (values[5] == "xdsl")
//						item.m_connection_type = ip_geo_item::xdsl;
//					else if (values[5] == "dsl")
//						item.m_connection_type = ip_geo_item::dsl;
//					else if (values[5] == "dialup")
//						item.m_connection_type = ip_geo_item::dialup;
//					else if (values[5] == "t1")
//						item.m_connection_type = ip_geo_item::t1;
//					else if (values[5] == "mobile")
//						item.m_connection_type = ip_geo_item::mobile;
//					else if (values[5] == "wireless")
//						item.m_connection_type = ip_geo_item::wireless;
//					else if (values[5] == "satellite")
//						item.m_connection_type = ip_geo_item::satellite;
//					else if (values[5] == "t3")
//						item.m_connection_type = ip_geo_item::t3;
//					else if (values[5] == "oc3")
//						item.m_connection_type = ip_geo_item::oc3;
//					else if (values[5] == "oc12")
//						item.m_connection_type = ip_geo_item::oc12;
//					else
//						cout << "Unknown connection type: '" << values[5] << "'" << endl;
//					//start_ip|end_ip|country|region|city|conn-speed|country-conf|region-conf|city-conf|metro-code|latitude|longitude|country-code|region-code|city-code|continent-code|two-letter-country|area-code|zip-code|gmt-offset|in-dst|zip-code-text|zip-country|isp-name|
//					//1.8.1.0|1.8.1.255|chn|11|beijing|broadband|5|4|4|-1|039.912|0116.389|156|10664|3036|4|cn|0|0|+800|n|100000|chn|knet techonlogy co. ltd.|
//					typedef pair<unsigned long, unsigned long> long_pair;
//					long_pair ip_pair(item.m_start_ip, item.m_end_ip);
//					geo_map.insert(pair<long_pair, ip_geo_item>(ip_pair, item));
//					counter++;
//					if (counter % 100000 == 0)
//						cout << counter << " records parsed" <<endl;
//				}
//				catch (std::exception &ex)
//				{
//					int i=0;
//				}
//			}
//		}
//	}
//	fclose(stream);
//	cout << "Loaded " << counter << " records." << endl;
//	cout << geo_map.size() << " records according to storage map" << endl;
//	cout << "Time to load: " << timer.elapsed() << endl;
//	{
//		ofstream output_stream("C:\\1\\ip_geo_data.txt", ios_base::out | ios_base::trunc);
//		text_oarchive ip_geo_archive(output_stream);
//		map <pair<unsigned long, unsigned long>, ip_geo_item>::const_iterator it_beg=geo_map.begin();
//		map <pair<unsigned long, unsigned long>, ip_geo_item>::const_iterator it_end=geo_map.end();
//		for (; it_beg != it_end; ++it_beg)
//		{
//			ip_geo_item *t = const_cast<ip_geo_item*>(&(it_beg->second));
//			ip_geo_archive << t;
//		}
//		output_stream.close();
//	}
//	{
//		ofstream output_stream1("C:\\1\\geo_data.txt", ios_base::out | ios_base::trunc);
//		text_oarchive geo_archive(output_stream1);
//		map	<unsigned int, geo_item>::const_iterator it_beg=iso3166_country_data::instance().get_country_map().begin();
//		map	<unsigned int, geo_item>::const_iterator it_end=iso3166_country_data::instance().get_country_map().end();
//		for (; it_beg != it_end; ++it_beg)
//		{
//			geo_item *t = const_cast<geo_item*>(&(it_beg->second));
//			geo_archive << t;
//		}
//		output_stream1.close();
//	}
//	getchar();
//	return 0;
//}

//uuid_t get_uuid()
//{
//	uuid_t uuid; 
//	SecureZeroMemory(&uuid,sizeof(UUID)); 
//	UuidCreate(&uuid);
//	return uuid;
//}
//bool std::less<uuid_t>::operator() (
//	const uuid_t &lhs, 
//	const uuid_t &rhs) const
//{
//	/*if (lhs.Data1 < rhs.Data1)
//		return true;
//	else if (lhs.Data2 < rhs.Data2)
//		return true;
//	else if (lhs.Data3 < rhs.Data3)
//		return true;
//	else if (lhs.Data4 < rhs.Data4)
//		return true;
//	else*/
//		return false;
//}
//
//bool operator<(
//	const uuid_t &lhs,
//	const uuid_t &rhs)
//{
//	if (lhs.Data1 < rhs.Data1)
//		return true;
//	else if (lhs.Data2 < rhs.Data2)
//		return true;
//	else if (lhs.Data3 < rhs.Data3)
//		return true;
//	else if (lhs.Data4 < rhs.Data4)
//		return true;
//	else
//		return false;
//}


int _tmain(int argc, _TCHAR* argv[])
{
	vector <ip_geo_item> ip_geo_map;
	{
		boost::timer timer;
		ifstream input_stream("C:\\1\\geo_data.txt");
		text_iarchive geo_archive(input_stream);
		unsigned int counter = 0;
		try
		{
			geo_item *t = NULL;
			for (;;)
			{
				geo_archive >> t;
				iso3166_country_data::instance().get_country_map().insert(pair<unsigned int, geo_item>(t->get_hash(), *t));
				counter++;
				if (counter % 10000 == 0)
					cout << counter << " geo records loaded" <<endl;
			}
		}
		catch (archive_exception &ex)
		{
			cout << ex.what() << endl;
			int j=1;
		}
		catch (std::exception &ex)
		{
			cout << ex.what() << endl;                                                                            
			int j=1;
		}
		cout << iso3166_country_data::instance().get_country_map().size() << " geo records loaded in " << timer.elapsed() << " seconds" << endl;
		input_stream.close();
	}
	{
		boost::timer timer;
		boost::uuids::basic_random_generator<boost::mt19937> gen;
		ifstream input_stream("C:\\1\\ip_geo_data.txt");
		text_iarchive ip_geo_archive(input_stream);
		unsigned int counter = 0;
		try
		{
			ip_geo_item *t = NULL;
			for (;;)
			{
				ip_geo_archive >> t;
				ip_geo_map.push_back(*t);
				counter++;
				if (counter % 100000 == 0)
					cout << counter << " ip-geo records loaded" <<endl;
			}
		}
		catch (archive_exception &ex)
		{
			cout << ex.what() << endl;
			int j=1;
		}
		catch (std::exception &ex)
		{
			cout << ex.what() << endl;                                                                            
			int j=1;
		}
		cout << ip_geo_map.size() << " ip-geo records loaded in " << timer.elapsed() << " seconds" << endl;
		input_stream.close();
	}
	size_t vec_size = (ip_geo_map.size()-1)/RAND_MAX;
	srand(time(NULL));
	size_t isr_count = 0;
	size_t miss = 0;
	getchar();
	cout << "Press Enter to continue" << endl;
	boost::timer timer;
	for (int i = 0; i < 1000000000; i++)
	{
		size_t random_pos = (rand()*vec_size);
		ip_geo_item ip_entry = ip_geo_map[random_pos];
		
			if (ip_entry.m_geo_item.m_country.get_numeric() == 392 ||
				ip_entry.m_geo_item.m_country.get_numeric() == 972)
			{
				isr_count++;
				/*ip_geo_map.erase(ip_geo_map..at(random_pos));
				ip_geo_map.push_back(ip_entry);*/
			}
		else
		{
			miss++;
		}
	}
	cout << isr_count << " entries of Japan found within " << timer.elapsed() << " seconds" << endl;
	cout << miss << " misses" << endl;
	getchar();
	return 0;
}
