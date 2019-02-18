#include <iostream>
#include <memory>
#include <boost/lockfree/queue.hpp>
#include <vector>
#include <unordered_map>
#include <map>

class BufferPool
{
public:
	BufferPool(size_t sizeParam, size_t alignmentParam, size_t initialSize) : size(sizeParam), alignment(std::align_val_t(alignmentParam))
	{
		Replenish(initialSize);
	}

	~BufferPool()
	{
		isStopping = true;
		while (!pool.empty()) {
			uint8_t* buffer;
			if (pool.pop(buffer)) {
				operator delete[](buffer, size, alignment);
			}
		}
	}

	std::shared_ptr<uint8_t> GetSharedBuffer()
	{
		if (isStopping) {
			return {};
		}
		if (pool.empty()) {
			Replenish(16);
		}
		uint8_t* buffer;
		while (!pool.pop(buffer)) {
			Replenish(16);
		}
		return std::shared_ptr<uint8_t>(buffer, [this](void* p) {
			if (isStopping) {
				operator delete[](p, size, alignment);
				return;
			}
			pool.push(p);
		});
	}

	auto GetUniqueBuffer()
	{
		static auto deleter = [this](void* p) {
			if (isStopping) {
				operator delete[](p, size, alignment);
				return;
			}
			pool.push(p);
		};

		if (isStopping) {
			return std::unique_ptr<uint8_t[], decltype(deleter)>(nullptr, deleter);
		}
		if (pool.empty()) {
			Replenish(16);
		}
		uint8_t* buffer;
		while (!pool.pop(buffer)) {
			Replenish(16);
		}
		return std::unique_ptr<uint8_t[], decltype(deleter)>(buffer, deleter);
	}

private:
	void Replenish(size_t amount)
	{
		for (auto i = 0ul; i < amount; ++i) {
			pool.push(new(alignment) uint8_t[size]);
		}
	}

	boost::lockfree::queue<void*, boost::lockfree::capacity<0xFFFF - 1>> pool;
	size_t size;
	std::align_val_t alignment;
	bool isStopping = false;
};

class BufferPoolManager
{
public:
	auto GetUniqueBuffer(size_t size, size_t alignment)
	{
		if (pools.find(std::tie(size, alignment)) == pools.end()) {
			pools.emplace(std::make_pair(std::make_tuple(size, alignment), std::make_unique<BufferPool>(size, alignment, 16)));
		}
		return pools.find(std::tie(size, alignment))->second->GetUniqueBuffer();
	}

	auto GetSharedBuffer(size_t size, size_t alignment)
	{
		if (pools.find(std::tie(size, alignment)) == pools.end()) {
			pools.emplace(std::make_pair(std::make_tuple(size, alignment), std::make_unique<BufferPool>(size, alignment, 16)));
		}
		return pools.find(std::tie(size, alignment))->second->GetSharedBuffer();
	}

private:
	std::map<std::tuple<size_t, size_t>, std::unique_ptr<BufferPool>> pools;
};

template<typename T>
T next_power2(T value)
{
	return 1 << ((sizeof(T) * 8) - __builtin_clz(value - 1));
}

int main()
{
	BufferPoolManager manager;
	{
		std::vector<std::shared_ptr<uint8_t>> buffers;
		for (auto i = 0; i < 1000; ++i) {

			buffers.emplace_back(manager.GetSharedBuffer(1024, 32));
		}
		for (auto i = 0; i < 1000; ++i) {

			buffers.emplace_back(manager.GetSharedBuffer(2048, 64));
		}
	}
	return 0;
}