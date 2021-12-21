#pragma once
#include <random>

template<typename Container>
Container createRandomData(size_t size)
{
	Container buffer;
	buffer.resize(size);

	std::seed_seq seq{rand(), static_cast<int>(time_t()), static_cast<int>(size),
					  static_cast<int>(reinterpret_cast<uint64_t>(buffer.data()))};
	seq.generate(buffer.begin(), buffer.end());
	return buffer;
}