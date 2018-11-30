//#include "../../Development/Tests/shared_state_test/Throughput.h"
#include <iostream>
#include <chrono>

#include <linux/fs.h>
#include <sys/ioctl.h>

#include <boost/asio/io_context.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/asio/steady_timer.hpp>

#include <boost/fiber/all.hpp>
#include <boost/fiber/mutex.hpp>
#include <boost/fiber/barrier.hpp>
#include <boost/fiber/algo/algorithm.hpp>
#include <boost/fiber/algo/work_stealing.hpp>
#include <random>
#include <future>
#include <aio.h>

namespace baio = boost::asio;

class IOContext : public std::enable_shared_from_this<IOContext>
{
	baio::io_context ioCtx;
	baio::executor_work_guard<baio::io_context::executor_type> work {baio::make_work_guard(ioCtx)};
public:
	void Init()
	{
		std::thread([self {shared_from_this()}]() {
			self->ioCtx.run();
		}).detach();
	}

	baio::io_context& GetContext()
	{
		return ioCtx;
	}
};

class DeadlineTimer : public std::enable_shared_from_this<DeadlineTimer>
{
	IOContext& ctx;
	baio::steady_timer timer;
public:
	explicit DeadlineTimer(IOContext& context) : ctx(context), timer(ctx.GetContext())
	{}

	template<typename Rep, typename Period, typename T>
	void TimedExecute(std::chrono::duration<Rep, Period> when, T&& func)
	{
		baio::spawn(ctx.GetContext(),
					[self {shared_from_this()}, when {std::move(when)}, func(std::move(func))](baio::yield_context yield) {
						boost::system::error_code ec;
						self->timer.expires_after(when);
						self->timer.async_wait(yield[ec]);
						if (ec != boost::asio::error::operation_aborted) {
							func();
						}
					});
	}

	void Reset()
	{
		timer.cancel();
	}
};

namespace bf = boost::fibers;

class GreenExecutor
{
	std::vector<std::thread> workers;
	bf::condition_variable_any cv;
	bf::mutex mtx;
	bf::barrier barrier;
public:
	GreenExecutor() : GreenExecutor(1)
	{
	}

	GreenExecutor(uint64_t concurrencyHint) : barrier {concurrencyHint}
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

	template<typename T>
	void PostWork(T&& functor)
	{
		bf::fiber f {std::move(functor)};
		f.detach();
	}

	~GreenExecutor()
	{
		cv.notify_all();
		for (auto& thread:workers) {
			thread.join();
		}
	}
};

class BlockDeviceIO : public std::enable_shared_from_this<BlockDeviceIO>
{
public:
	explicit BlockDeviceIO(const std::string& deviceName)
	{
		blockFD = open(deviceName.c_str(), O_RDONLY/*O_RDWR*/);

		uint64_t size = 0;
		unsigned int blockSize = 0;

		if (blockFD < 0 || ioctl(blockFD, BLKGETSIZE64, &size) < 0 || ioctl(blockFD, BLKSSZGET, &blockSize) < 0) {
			auto errMsg = std::strerror(errno);
			if (blockFD >= 0) {
				close(blockFD);
			}
			throw std::runtime_error(errMsg);
		}
	}

	auto ReadAsync(size_t offset, size_t size)
	{
		return bf::async([self {shared_from_this()}, offset, size]() { return self->Read(offset, size); });
	}

	auto WriteAsync(size_t offset, const std::vector<uint8_t>& buff)
	{
		return bf::async([self {shared_from_this()}, offset, &buff]() { return self->Write(offset, buff); });
	}

	~BlockDeviceIO()
	{
		if (close(blockFD) != 0) {
			std::terminate();
		}
	}

private:
	std::vector<uint8_t> Read(size_t offset, size_t size)
	{
		std::vector<uint8_t> retVal(size);
		aiocb cb = {0};

		cb.aio_nbytes = size;
		cb.aio_fildes = blockFD;
		cb.aio_offset = offset;
		cb.aio_buf = retVal.data();

		if (aio_read(&cb) == -1) {
			throw std::runtime_error(std::strerror(aio_error(&cb)));
		}

		while (aio_error(&cb) == EINPROGRESS) {
			boost::this_fiber::yield();
		}

		auto numBytes = aio_return(&cb);

		if (numBytes < 0) {
			throw std::runtime_error(std::strerror(aio_error(&cb)));
		}
		retVal.resize(numBytes);
		return retVal;
	}

	bool Write(size_t offset, const std::vector<uint8_t>& buff)
	{
		aiocb cb = {0};

		cb.aio_nbytes = buff.size();
		cb.aio_fildes = blockFD;
		cb.aio_offset = offset;
		cb.aio_buf = const_cast<uint8_t*>(buff.data());

		if (aio_write(&cb) == -1) {
			throw std::runtime_error(std::strerror(aio_error(&cb)));
		}

		while (aio_error(&cb) == EINPROGRESS) {
			boost::this_fiber::yield();
		}

		auto numBytes = aio_return(&cb);

		if (numBytes < 0) {
			throw std::runtime_error(std::strerror(aio_error(&cb)));
		}
		return true;
	}

private:
	int blockFD;
};

int main()
{
	GreenExecutor exec(2);
	constexpr size_t dataSize = 4096;
	for (auto i = 0; i < 10; ++i) {
		size_t ioDepth = pow(2, i);
		std::vector<std::shared_ptr<BlockDeviceIO>> block(ioDepth, std::make_shared<BlockDeviceIO>("/dev/nvme0n1"));
		auto start = std::chrono::steady_clock::now();
		size_t size = 0;
		size_t pos = 0;
		for (auto j = 0ul; j < 1'000'000 / ioDepth; ++j) {
			std::vector<decltype(block[0]->ReadAsync(0, dataSize))> ios;
			ios.reserve(ioDepth);
			for (auto k = 0ul; k < ioDepth; ++k) {
				ios.emplace_back(block[k]->ReadAsync(pos, dataSize));
				pos += dataSize;
			}
			for (auto& res:ios) {
				size += res.get().size();
			}
		}
		auto end = std::chrono::steady_clock::now();
		std::cout << size_t(double(size) / std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() * 1000 / 1024 / 1024)
				  << "MB/s" << " for IO depth " << ioDepth << std::endl;
	}
	//	std::vector<uint8_t> buff(1024);
	//	std::random_device rd;  //Will be used to obtain a seed for the random number engine
	//	std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
	//	std::uniform_int_distribution<> dis(0x41, 0x7A);
	//	std::generate(buff.begin(), buff.end(), [&dis, &gen]() { return dis(gen); });
	//	auto writeRes = block->WriteAsync(0, buff);
	//	std::cout << "Write result: " << writeRes.get() << std::endl;
	//	auto res = block->ReadAsync(0, 1024);
	//	auto resBuff = res.get();
	//	std::cout << "Size: " << resBuff.size() << std::endl;
	//	std::cout << std::string(reinterpret_cast<char*>(resBuff.data()), resBuff.size()) << std::endl;
	//	auto ctx = std::make_shared<IOContext>();
	//	ctx->Init();
	//	{
	//		auto timer = std::make_shared<DeadlineTimer>(*(ctx.get()));
	//		auto timer1 = std::make_shared<DeadlineTimer>(*(ctx.get()));
	//		std::string msg = "Bang!";
	//		timer->TimedExecute(std::chrono::seconds(1), [&msg]() {
	//			std::this_thread::sleep_for(std::chrono::seconds(10));
	//			std::cout << msg << std::endl;
	//		});
	//		std::this_thread::sleep_for(std::chrono::seconds(2));
	//		std::cout << "Before timer destruction" << std::endl;
	//	}
	//timer->Reset();
	//std::cout << "After timer destruction" << std::endl;
	//timer1->TimedExecute(std::chrono::seconds(1), []() { std::cout << "Bung!" << std::endl; });
	//	GreenExecutor executor;
	//	std::this_thread::sleep_for(std::chrono::seconds(1));
	//	int i = 0;
	//	for(auto j=0ul;j<10;++j) {
	//		executor.PostWork([idx {++i}]() {
	//			auto res = pow(sqrt(sin(cos(tan(idx)))), M_1_PI);
	//			std::cout << idx << " - " << res << std::endl;
	//		});
	//	}
	//	executor.PostWork([idx {++i}]() {
	//		auto res = pow(sqrt(sin(cos(tan(idx)))), M_1_PI);
	//		std::cout << idx << " - " << res << std::endl;
	//
	//	});
	//	executor.PostWork([idx {++i}]() {
	//		auto res = pow(sqrt(sin(cos(tan(idx)))), M_1_PI);
	//		std::cout << idx << " - " << res << std::endl;
	//
	//	});
	//	executor.PostWork([idx {++i}]() {
	//		auto res = pow(sqrt(sin(cos(tan(idx)))), M_1_PI);
	//		std::cout << idx << " - " << res << std::endl;
	//
	//	});
	//	while (true) {
	//		std::this_thread::yield();
	//		//boost::this_fiber::yield();
	//	}
	return 0;
}
