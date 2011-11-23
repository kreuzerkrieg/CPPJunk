// string_test.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

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
template <typename T>
void fill_vector(std::vector<T> &vec)
{
	for (int i = 0; i < VEC_SIZE; i++)
	{
		vec.push_back(boost::lexical_cast<T>(rand()));
	}
}

template <typename T>
void vector_test(std::vector<T> &tmp)
{
	std::cout << "Testing vector of " << typeid(T).name() << " type" << std::endl << std::endl;
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
		std::vector<T> tmp_loc;
		tmp_loc.reserve(VEC_SIZE);
		fill_vector(tmp_loc);
	}
	std::cout << t.elapsed() << " to clear vector with allocating on stack new vector" << std::endl;
}

int _tmain(int argc, _TCHAR* argv[])
{
	srand(time(NULL));
	string_test();
	std::vector <int> int_vec;
	vector_test(int_vec);

	std::vector <std::string> str_vec;
	vector_test(str_vec);
	return 0;
}

