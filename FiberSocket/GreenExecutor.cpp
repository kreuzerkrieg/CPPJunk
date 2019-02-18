#include "GreenExecutor.h"

GreenExecutor::GreenExecutor() : GreenExecutor(1)
{
}

GreenExecutor::GreenExecutor(uint64_t concurrencyHint) : barrier{concurrencyHint}
{

	bf::use_scheduling_algorithm<bf::algo::work_stealing>(concurrencyHint);

	for (auto i = 0ul; i < concurrencyHint; ++i) {
		workers.emplace_back(std::thread([concurrencyHint, this] {
			bf::use_scheduling_algorithm<bf::algo::work_stealing>(concurrencyHint);
			// wait till all threads joining the work stealing have been registered
			barrier.wait();
			mtx.lock();
			// suspend main-fiber from the worker thread
			cv.wait(mtx);
			mtx.unlock();
		}));
		// wait till all threads have been registered the scheduling algorithm
		//barrier.wait();
	}
}

GreenExecutor::~GreenExecutor()
{
	cv.notify_all();
	for (auto& thread:workers) {
		thread.join();
	}
}
