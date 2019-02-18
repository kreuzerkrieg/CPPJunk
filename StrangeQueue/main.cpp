#include <iostream>
#include <vector>
#include <future>
#include <deque>
#include "Queue.h"

struct Foo
{
	Foo() = default;

	explicit Foo(int32_t i) : _1(i)
	{}

	int32_t _1 = -1;
	int64_t _2 = 1;
	double _3 = 2;
	std::string _4;
	std::string _5;
	std::vector<double> _6;
	std::promise<void> prom;
};

int main()
{
	{
		Queue<Foo> qFoo;
		for (auto i = 0; i < 1000; ++i) {
			Foo foo;
			foo._1 = i;
			qFoo.Push(std::move(foo));
		}
		for (auto i = 0; i < 1000; ++i) {
			Foo foo;
			qFoo.Pop(foo);
			if (foo._1 < 0) {
				std::cout << "Bang!" << std::endl;
			}
		}
	}

	{
		Queue<Foo*> qFoo;
		for (auto i = 0; i < 1000; ++i) {
			qFoo.Push(new Foo(i));
		}
		for (auto i = 0; i < 1000; ++i) {
			Foo foo;
			qFoo.Pop(foo);
			if (foo._1 < 0) {
				std::cout << "Bang!" << std::endl;
			}
		}
	}
	{
		Queue<Foo*> qFoo;
		for (auto i = 0; i < 1000; ++i) {
			auto foo = new Foo;
			foo->_1 = i;
			qFoo.Push(foo);
		}
		for (auto i = 0; i < 1000; ++i) {
			Foo* foo;
			qFoo.Pop(foo);
			if (foo->_1 < 0) {
				std::cout << "Bang!" << std::endl;
			}
			delete foo;
		}
	}

	std::deque<std::string> q;
	q.push_back("abc");
	q.pop_back();
	return 0;
}