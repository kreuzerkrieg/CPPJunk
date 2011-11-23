// string_test.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <string>
#include <vector>
#include <iostream>
#include <boost\timer.hpp>

void string_test()
{
	std::string tmp;
	boost::timer t;
	t.restart();
	for (int i = 0; i < 1000000000; i++)
	{
		tmp = "sjkdfh ksjhf kjdsfhj ksdjhfj skjdh";
		tmp = "";
	}
	std::cout << t.elapsed() << " to nullify string with assignment" << std::endl;
	t.restart();
	for (int i = 0; i < 1000000000; i++)
	{
		tmp = "sjkdfh ksjhf kjdsfhj ksdjhfj skjdh";
		tmp.resize(0);
	}
	std::cout << t.elapsed() << " to nullify string with \"resize(0)\"" << std::endl;
	t.restart();
	for (int i = 0; i < 1000000000; i++)
	{
		tmp = "sjkdfh ksjhf kjdsfhj ksdjhfj skjdh";
		tmp.clear();
	}
	std::cout << t.elapsed() << " to nullify string with \"clear()\"" << std::endl;
}

#define VEC_SIZE 10
void fill_vector(std::vector<int> &vec)
{
	for (int i = 0; i < VEC_SIZE; i++)
	{
		vec.push_back(rand());
	}
}

void vector_test()
{
	std::vector<int> tmp;
	tmp.reserve(VEC_SIZE);
	boost::timer t;
	t.restart();
	for (int i = 0; i < 100000000; i++)
	{
		fill_vector(tmp);
		tmp.resize(0);
	}
	std::cout << t.elapsed() << " to clear vector with \"resize(0)\"" << std::endl;
	t.restart();
	for (int i = 0; i < 100000000; i++)
	{
		fill_vector(tmp);
		tmp.clear();
	}
	std::cout << t.elapsed() << " to clear vector with \"clear()\"" << std::endl;
	t.restart();
	for (int i = 0; i < 100000000; i++)
	{
		fill_vector(tmp);
		tmp.erase(tmp.begin(), tmp.end());
	}
	std::cout << t.elapsed() << " to clear vector with \"erase(begin(), end())\"" << std::endl;
	t.restart();
	for (int i = 0; i < 100000000; i++)
	{
		std::vector<int> tmp_loc;
		tmp_loc.reserve(VEC_SIZE);
		fill_vector(tmp_loc);
	}
	std::cout << t.elapsed() << " to clear vector with allocating on stack new vector" << std::endl;
}

int _tmain(int argc, _TCHAR* argv[])
{
	srand(time(NULL));
	string_test();
	vector_test();
	return 0;
}

