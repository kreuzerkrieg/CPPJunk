#include "Crc32.h"
#include "utils/BufferUtils.h"
#include <benchmark/benchmark.h>
#include <zlib.h>

static void AdlerCRC32(benchmark::State& state)
{
	auto buff = createRandomData<std::vector<unsigned char>>(state.range(0));
	uint64_t sum = 0;
	for (auto _: state) {
		auto result = crc32(0, buff.data(), buff.size());
		benchmark::DoNotOptimize(sum += result);
	}
	state.SetBytesProcessed(int64_t(state.iterations()) * int64_t(state.range(0)));
}
static void GenericCRC32(benchmark::State& state)
{
	auto buff = createRandomData<std::vector<unsigned char>>(state.range(0));
	uint64_t sum = 0;
	for (auto _: state) {
		auto result = CRC32::Generic::compute(0, buff.data(), buff.size());
		benchmark::DoNotOptimize(sum += result);
	}
	state.SetBytesProcessed(int64_t(state.iterations()) * int64_t(state.range(0)));
}
static void GenericExCRC32(benchmark::State& state)
{
	auto buff = createRandomData<std::vector<unsigned char>>(state.range(0));
	uint64_t sum = 0;
	for (auto _: state) {
		auto result = CRC32::Generic::compute_ex(0, buff.data(), buff.size());
		benchmark::DoNotOptimize(sum += result);
	}
	state.SetBytesProcessed(int64_t(state.iterations()) * int64_t(state.range(0)));
}
static void GenericWide4CRC32(benchmark::State& state)
{
	auto buff = createRandomData<std::vector<unsigned char>>(state.range(0));
	uint64_t sum = 0;
	for (auto _: state) {
		auto result = CRC32::Generic::compute_wide<4>(0, buff.data(), buff.size());
		benchmark::DoNotOptimize(sum += result);
	}
	state.SetBytesProcessed(int64_t(state.iterations()) * int64_t(state.range(0)));
}
static void GenericWide8CRC32(benchmark::State& state)
{
	auto buff = createRandomData<std::vector<unsigned char>>(state.range(0));
	uint64_t sum = 0;
	for (auto _: state) {
		auto result = CRC32::Generic::compute_wide<8>(0, buff.data(), buff.size());
		benchmark::DoNotOptimize(sum += result);
	}
	state.SetBytesProcessed(int64_t(state.iterations()) * int64_t(state.range(0)));
}
static void GenericWide16CRC32(benchmark::State& state)
{
	auto buff = createRandomData<std::vector<unsigned char>>(state.range(0));
	uint64_t sum = 0;
	for (auto _: state) {
		auto result = CRC32::Generic::compute_wide<16>(0, buff.data(), buff.size());
		benchmark::DoNotOptimize(sum += result);
	}
	state.SetBytesProcessed(int64_t(state.iterations()) * int64_t(state.range(0)));
}
static void GenericWide24CRC32(benchmark::State& state)
{
	auto buff = createRandomData<std::vector<unsigned char>>(state.range(0));
	uint64_t sum = 0;
	for (auto _: state) {
		auto result = CRC32::Generic::compute_wide<24>(0, buff.data(), buff.size());
		benchmark::DoNotOptimize(sum += result);
	}
	state.SetBytesProcessed(int64_t(state.iterations()) * int64_t(state.range(0)));
}
static void IntrinsicCRC32(benchmark::State& state)
{
	auto buff = createRandomData<std::vector<unsigned char>>(state.range(0));
	uint64_t sum = 0;
	for (auto _: state) {
		auto result = CRC32::Intrinsic::compute(0, buff.data(), buff.size());
		benchmark::DoNotOptimize(sum += result);
	}
	state.SetBytesProcessed(int64_t(state.iterations()) * int64_t(state.range(0)));
}

BENCHMARK(AdlerCRC32)->Name("Adler CRC32")->RangeMultiplier(8)->Range(1, 1 << 20);
/*BENCHMARK(GenericCRC32)->Name("std::accumulate CRC32")->RangeMultiplier(8)->Range(1, 1 << 20);
BENCHMARK(GenericWide4CRC32)->Name("Generic CRC32 4 bytes")->RangeMultiplier(8)->Range(1, 1 << 20);
BENCHMARK(GenericWide8CRC32)->Name("Generic CRC32 8 bytes")->RangeMultiplier(8)->Range(1, 1 << 20);
BENCHMARK(GenericWide16CRC32)->Name("Generic CRC32 16 bytes")->RangeMultiplier(8)->Range(1, 1 << 20);
BENCHMARK(GenericWide24CRC32)->Name("Generic CRC32 24 bytes")->RangeMultiplier(8)->Range(1, 1 << 20);*/
BENCHMARK(GenericExCRC32)->Name("Generic CRC32 32 bytes")->RangeMultiplier(8)->Range(1, 1 << 20);
BENCHMARK(IntrinsicCRC32)->Name("Intrinsic CRC32")->RangeMultiplier(8)->Range(1, 1 << 20);

BENCHMARK_MAIN();