// no_paging_allocator.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "np_allocator.h"
#include <vector>
#include <iostream>
#include <algorithm>
#include <string>
#include <ctime>
using namespace std;

const int size = 10000;

struct foo
{
	foo(const string &s = "Hello world", int i = 0):
s_(s),
	i_(i)
{
}
string s_;
int i_;
bool operator <(const foo& other)const{return i_<other.i_;}
};

foo generator()
{
	return foo("Hello world", rand());
}

int _tmain(int argc, _TCHAR* argv[])
{
	srand(983274892);
	cout.precision(10); 
	cout<<"***Tests with <vector>***"<<endl;
	vector<foo, np_allocator<foo> > ss_vector(size);
	vector<foo> default_vector(size);
	sort(ss_vector.begin(), ss_vector.end());
	sort(default_vector.begin(), default_vector.end());


	cout<<"***Tests with <list>***"<<endl;
	list<foo, np_allocator<foo> > ss_list(size);
	list<foo> default_list(size);
	ss_list.sort();
	default_list.sort();
	cout<<"Press <Enter>..."<<endl;
	char c;
	cin.getline(&c, 1);
	return 0;
}

