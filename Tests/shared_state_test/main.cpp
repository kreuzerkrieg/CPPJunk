#define NONIUS_RUNNER

#include <nonius.h++>
#include <iostream>
#include <boost/fiber/future/future.hpp>
#include <boost/fiber/future/promise.hpp>
//#include <boost/fiber/barrier.hpp>
#include <boost/fiber/unbuffered_channel.hpp>
#include "Throughput.h"


//struct boost_task {
////    boost_task() = default;
////
////    boost_task(boost_task &&rhs) = default;
//
//    //boost::fibers::promise<void> promise;
//    size_t _1 = 0;
//    size_t _2 = 0;
//    size_t _3 = 0;
//    size_t _4 = 0;
//};

struct lambder
{
	decltype(auto) createTask(boost::fibers::future<void>& fut)
	{
		boost::fibers::promise<void> prom;
		fut = prom.get_future();
		return [p{std::move(prom)}]()mutable { p.set_value(); };
	}
};

//struct bar {
//    bar() {
//        using task= decltype(bar::createLambda());
//    }
//
//    decltype(auto) createLambda() {
//        std::promise<void> promise;
//        auto future = promise.get_future();
//        return test([promise{std::move(promise)}]() mutable { promise.set_value(); });
//    }
//
//    void foo() {
//        _vec.emplace_back(createLambda());
//    }
//
//    //using task = decltype(bar::createLambda());
//    std::vector<test<decltype(bar::createLambda())>::type> _vec;
//};

struct fiber_worker
{
	fiber_worker()
	{
		wthread = std::thread([self{this}]() {
			for (int i = 0; i < 16; ++i) {
				boost::fibers::fiber{[self]() {
					while (!self->ch.is_closed()) {
						(self->ch.value_pop())();
					}
				}}.detach();
			}
			while (!self->ch.is_closed()) {
				(self->ch.value_pop())();
			}
		});
	}

	boost::fibers::future<void> submit()
	{
		boost::fibers::future<void> retVal;
		lambder l;
		ch.push(l.createTask(retVal));
		return retVal;
	}

	~fiber_worker()
	{
		ch.close();
		wthread.join();
	}

	std::thread wthread;
	static boost::fibers::future<void> future;
	struct task
	{
		using type = decltype((std::declval<lambder&>().createTask(future)));
	};
	boost::fibers::unbuffered_channel<task::type> ch;
};

//
//
//
////NONIUS_BENCHMARK("future/promise in the same thread", [](nonius::chronometer meter) {
////    meter.measure([] {
////        std::promise<void> promise;
////        auto future = promise.get_future();
////        promise.set_value();
////        future.get();
////    });
////})
////
NONIUS_BENCHMARK("future/promise", [](nonius::chronometer meter) {
	meter.measure([] {
		std::promise<void> promise;

		auto future = promise.get_future();
		task t;
		t.promise = std::move(promise);
		wk.submit(std::move(t));
		future.get();
	});
})
////
////NONIUS_BENCHMARK("boost fiber future/promise", [](nonius::chronometer meter) {
////    meter.measure([] {
////        boost::fibers::promise<void> promise;
////
////        auto future = promise.get_future();
////        boost_task t;
////        t.promise = std::move(promise);
////        bwk.submit(std::move(t));
////        future.get();
////    });
////})
////
////NONIUS_BENCHMARK("boost thread future/promise", [](nonius::chronometer meter) {
////    meter.measure([] {
////        boost::promise<void> promise;
////
////        auto future = promise.get_future();
////        boost_t_task t;
////        t.promise = std::move(promise);
////        btwk.submit(std::move(t));
////        future.get();
////    });
////})
////
////NONIUS_BENCHMARK("dumb future/promise", [](nonius::chronometer meter) {
////    meter.measure([] {
////        dumb_task t;
////        auto future = t.promise.get_future();
////        wk3.submit(std::move(t));
////        future.get();
////    });
////})
////
////NONIUS_BENCHMARK("dumb smart future/promise", [](nonius::chronometer meter) {
////    meter.measure([] {
////        dumb_task1 t;
////        auto future = t.promise.get_future();
////        wk4.submit(std::move(t));
////        future.get();
////    });
////})
////NONIUS_BENCHMARK("future/promise with dynamic allocation", [](nonius::chronometer meter) {
////    meter.measure([] {
////        std::promise<void> promise;
////        auto future = promise.get_future();
////        auto t = new task;
////        t->promise = std::move(promise);
////        wk1.submit(t);
////        future.get();
////    });
////})
////
////NONIUS_BENCHMARK("future/promise with lockless queue", [](nonius::chronometer meter) {
////    meter.measure([] {
////        std::promise<void> promise;
////        auto future = promise.get_future();
////        auto t = new task;
////        t->promise = std::move(promise);
////        wk1.submit(t);
////        future.get();
////    });
////})
//
//fiber_worker bwk;
//
//int main() {
//    std::cout
//            << "===================================== ..:: Boost.Fibers ::.. ====================================="
//            << std::endl;
//    {
//        auto lambda = []() {
//            bwk.submit().get();
//            return 1;
//        };
//
//        Throughput<boost::fibers::fiber, decltype(lambda)> test(std::move(lambda), 1);
//        test.Start();
//
//        test.Stop();
//    }
//    std::cout
//            << "===================================== ..:: std::future/std::promise ::.. ====================================="
//            << std::endl;
//    {
//        auto lambda = []() {
//            std::promise<void> promise;
//
//            auto future = promise.get_future();
//            task t;
//            t.promise = std::move(promise);
//            wk.submit(std::move(t));
//            future.get();
//            return 1;
//        };
//        Throughput<std::thread, decltype(lambda)> test(std::move(lambda), 1);
//        test.Start();
//
//        test.Stop();
//    }
//    std::cout
//            << "===================================== ..:: std::future/std::promise ::.. ====================================="
//            << std::endl;
//    {
//        auto lambda = []() {
//            std::promise<void> promise;
//
//            auto future = promise.get_future();
//            task t;
//            t.promise = std::move(promise);
//            wk.submit(std::move(t));
//            future.get();
//            return 1;
//        };
//        Throughput<std::thread, decltype(lambda)> test(std::move(lambda), 1000, 1, 64, 1);
//        test.Start();
//
//        test.Stop();
//    }
//    std::cout
//            << "===================================== ..:: std::future/std::promise ::.. ====================================="
//            << std::endl;
//    {
//        auto lambda = []() {
//            std::promise<void> promise;
//
//            auto future = promise.get_future();
//            task t;
//            t.promise = std::move(promise);
//            wk.submit(std::move(t));
//            future.get();
//            return 1;
//        };
//        Throughput<std::thread, decltype(lambda)> test(std::move(lambda), 1000, 64, 1);
//        test.Start();
//
//        test.Stop();
//    }
//    std::cout
//            << "===================================== ..:: boost::fibers::promise ::.. ====================================="
//            << std::endl;
//    {
//        auto lambda = []() {
//            boost::fibers::promise<void> promise;
//
//            auto future = promise.get_future();
//            boost_task t;
//            t.promise = std::move(promise);
//            bwk.submit(std::move(t));
//            future.get();
//            return 1;
//        };
//        Throughput<decltype(lambda)> test(std::move(lambda), 2);
//        test.Start();
//        test.Stop();
//    }
//    std::cout << "===================================== ..:: boost::promise ::.. ====================================="
//              << std::endl;
//    {
//        Throughput test([]() {
//            boost::promise<void> promise;
//
//            auto future = promise.get_future();
//            boost_t_task t;
//            t.promise = std::move(promise);
//            btwk.submit(std::move(t));
//            future.get();
//            return 1;
//        }, 2);
//        test.Start();
//        test.Stop();
//    }
//    std::cout << "===================================== ..:: promisette ::.. ====================================="
//              << std::endl;
//    {
//        auto lambda = []() {
//            promisette promise;
//
//            auto future = promise.get_future();
//            dumb_task t;
//            t.promise = std::move(promise);
//            wk3.submit(std::move(t));
//            future.get();
//            return 1;
//        };
//        Throughput<std::thread, decltype(lambda)> test(std::move(lambda), 1);
//        test.Start();
//        test.Stop();
//    }
//    std::cout << "===================================== ..:: promisette1 ::.. ====================================="
//              << std::endl;
//    {
//        auto lambda = []() {
//            promisette1 promise;
//
//            auto future = promise.get_future();
//            dumb_task1 t;
//            t.promise = std::move(promise);
//            wk4.submit(std::move(t));
//            future.get();
//            return 1;
//        };
//        Throughput<std::thread, decltype(lambda)> test(std::move(lambda), 1);
//        test.Start();
//        test.Stop();
//    }
//}