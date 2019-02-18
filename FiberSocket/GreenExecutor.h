#pragma once

#include <boost/fiber/all.hpp>
#include <boost/fiber/mutex.hpp>
#include <boost/fiber/barrier.hpp>
#include <boost/fiber/algo/algorithm.hpp>
#include <boost/fiber/algo/work_stealing.hpp>

namespace bf = boost::fibers;

class GreenExecutor
{
	std::vector<std::thread> workers;
	bf::condition_variable_any cv;
	bf::mutex mtx;
	bf::barrier barrier;
public:
	GreenExecutor();

	explicit GreenExecutor(uint64_t concurrencyHint);

	template<typename T>
	void PostWork(T&& functor)
	{
		bf::fiber f{std::move(functor)};
		f.detach();
	}

	~GreenExecutor();
};
