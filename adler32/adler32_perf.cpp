#include "Adler32Generic.h"
#include "utils/BufferUtils.h"
#include <zlib.h>
#include <benchmark/benchmark.h>

static void OriginalAdler32(benchmark::State& state)
{
	auto buff = createRandomData<std::vector<unsigned char>>(state.range(0));
	uint64_t sum = 0;
	for (auto _: state) {
		auto result = adler32(0, buff.data(), buff.size());
		benchmark::DoNotOptimize(sum += result);
	}
	state.SetBytesProcessed(int64_t(state.iterations()) * int64_t(state.range(0)));
}
static void GenericAdler32(benchmark::State& state)
{
	auto buff = createRandomData<std::vector<unsigned char>>(state.range(0));
	uint64_t sum = 0;
	for (auto _: state) {
		auto result = Adler32::Generic::compute(0, buff.data(), buff.size());
		benchmark::DoNotOptimize(sum += result);
	}
	state.SetBytesProcessed(int64_t(state.iterations()) * int64_t(state.range(0)));
}
static void SIMDAdler32(benchmark::State& state)
{
	auto buff = createRandomData<std::vector<unsigned char>>(state.range(0));
	uint64_t sum = 0;
	for (auto _: state) {
		auto result = Adler32::SIMD::compute(0, buff.data(), buff.size());
		benchmark::DoNotOptimize(sum += result);
	}
	state.SetBytesProcessed(int64_t(state.iterations()) * int64_t(state.range(0)));
}

/*
BENCHMARK(OriginalAdler32)->Name("Original Adler32")->RangeMultiplier(8)->Range(1, 1 << 20);
BENCHMARK(GenericAdler32)->Name("Generic Adler32")->RangeMultiplier(8)->Range(1, 1 << 20);
BENCHMARK(SIMDAdler32)->Name("SIMD Adler32")->RangeMultiplier(8)->Range(1, 1 << 20);*/
