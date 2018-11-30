#include <iostream>

struct foo
{
	std::string bar(int i)
	{ return std::to_string(i); }
};

int main()
{
	foo f;
	std::cout << f.bar(42) << std::endl;
	return 0;
}