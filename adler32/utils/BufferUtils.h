#pragma once
#include <random>

template<typename Container>
Container createRandomData(size_t size)
{
	Container buffer;
	buffer.resize(size);
	std::random_device rd;
	std::mt19937 gen(rd());
	std::seed_seq seq{std::uniform_int_distribution<size_t>()(gen), static_cast<size_t>(time_t()), size,
					  reinterpret_cast<size_t>(buffer.data())};
	seq.generate(buffer.begin(), buffer.end());
	return buffer;
}