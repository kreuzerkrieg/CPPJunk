#include "Crc32.h"
#include "utils/BufferUtils.h"
#include <benchmark/benchmark.h>
#include <zlib.h>
namespace Crc32Impl = CRC32::Generic;

/*NONIUS_BENCHMARK("Compute original crc32 1KiB", [](nonius::chronometer cm) {
	auto buff = createRandomData<std::vector<unsigned char>>(1024);
	cm.measure([&](int) { crc32(0, buff.data(), buff.size()); });
})

NONIUS_BENCHMARK("Compute crc32 1KiB", [](nonius::chronometer cm) {
	auto buff = createRandomData<std::vector<unsigned char>>(1024);
	cm.measure([&](int) { Crc32Impl::compute(0, buff.data(), buff.size()); });
})

NONIUS_BENCHMARK("Compute crc32 x 4 bytes 1KiB", [](nonius::chronometer cm) {
	auto buff = createRandomData<std::vector<unsigned char>>(1024);
	cm.measure([&](int) { Crc32Impl::computex32(0, buff.data(), buff.size()); });
})*/
static void BM_AdlerCRC32(benchmark::State& state)
{
	auto buff = createRandomData<std::vector<unsigned char>>(1024 * 1024);
	for (auto _: state)
		crc32(0, buff.data(), buff.size());
}
static void BM_GenericCRC32(benchmark::State& state)
{
	auto buff = createRandomData<std::vector<unsigned char>>(1024 * 1024);
	for (auto _: state)
		Crc32Impl::compute(0, buff.data(), buff.size());
}
static void BM_Genericx8CRC32(benchmark::State& state)
{
	auto buff = createRandomData<std::vector<unsigned char>>(1024 * 1024);
	uint64_t sum = 0;
	for (auto _: state) {
		auto result = Crc32Impl::compute_ex(0, buff.data(), buff.size());
		benchmark::DoNotOptimize(sum += result);
	}
}

static void BM_GenericCRC32_256_1(benchmark::State& state)
{
	auto buff = createRandomData<std::vector<unsigned char>>(1024 * 1024);
	uint64_t sum = 0;
	for (auto _: state) {
		auto result = Crc32Impl::append_table_256_1(0, buff.data(), buff.size());
		benchmark::DoNotOptimize(sum += result);
	}
}

static void BM_GenericCRC32_256_2(benchmark::State& state)
{
	auto buff = createRandomData<std::vector<unsigned char>>(1024 * 1024);
	uint64_t sum = 0;
	for (auto _: state) {
		auto result = Crc32Impl::append_table_256_2(0, buff.data(), buff.size());
		benchmark::DoNotOptimize(sum += result);
	}
}

static void BM_GenericCRC32_256_3(benchmark::State& state)
{
	auto buff = createRandomData<std::vector<unsigned char>>(1024 * 1024);
	uint64_t sum = 0;
	for (auto _: state) {
		auto result = Crc32Impl::append_table_256_3(0, buff.data(), buff.size());
		benchmark::DoNotOptimize(sum += result);
	}
}

static void BM_GenericCRC32Tmpl(benchmark::State& state)
{
	auto buff = createRandomData<std::vector<unsigned char>>(1024 * 1024);
	uint64_t sum = 0;
	for (auto _: state) {
		auto result = Crc32Impl::append_table_32(0, buff.data(), buff.size());
		benchmark::DoNotOptimize(sum += result);
	}
}

BENCHMARK(BM_AdlerCRC32);
BENCHMARK(BM_GenericCRC32);
BENCHMARK(BM_Genericx8CRC32);
BENCHMARK(BM_GenericCRC32_256_1);
BENCHMARK(BM_GenericCRC32_256_2);
BENCHMARK(BM_GenericCRC32_256_3);
BENCHMARK(BM_GenericCRC32Tmpl);

/*
NONIUS_BENCHMARK("Compute original crc32 1 byte", [](nonius::chronometer cm) {
	auto buff = createRandomData<std::vector<unsigned char>>(1);
	cm.measure([&](int) { crc32(0, buff.data(), buff.size()); });
})

NONIUS_BENCHMARK("Compute crc32 1 byte", [](nonius::chronometer cm) {
	auto buff = createRandomData<std::vector<unsigned char>>(1);
	cm.measure([&](int) { Crc32Impl::compute(0, buff.data(), buff.size()); });
})

NONIUS_BENCHMARK("Compute original crc32 15 bytes", [](nonius::chronometer cm) {
	auto buff = createRandomData<std::vector<unsigned char>>(15);
	cm.measure([&](int) { crc32(0, buff.data(), buff.size()); });
})

NONIUS_BENCHMARK("Compute crc32 15 bytes", [](nonius::chronometer cm) {
	auto buff = createRandomData<std::vector<unsigned char>>(15);
	cm.measure([&](int) { Crc32Impl::compute(0, buff.data(), buff.size()); });
})*/

/*
NONIUS_BENCHMARK("Combine original crc32 1MiB", [](nonius::chronometer cm) {
	auto buff = createRandomData<std::vector<unsigned char>>(1024 * 1024);
	uint32_t adler = crc32(0, buff.data(), buff.size());
	cm.measure([&](int) {
		for (int i = 0; i < 3; ++i) {
			adler = crc32_combine(adler, adler + i, buff.size());
		}
	});
})

NONIUS_BENCHMARK("Combine crc32 1MiB", [](nonius::chronometer cm) {
	auto buff = createRandomData<std::vector<unsigned char>>(1024 * 1024);
	uint32_t crc32 = Crc32Impl::compute(crc32, buff.data(), buff.size());
	cm.measure([&](int) {
		for (int i = 0; i < 3; ++i) {
			crc32 = Crc32Impl::combine(crc32, crc32 + i, buff.size());
		}
	});
})
*/
BENCHMARK_MAIN();