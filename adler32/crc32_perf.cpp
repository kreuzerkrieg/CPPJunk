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
static void AdlerCRC32(benchmark::State& state)
{
	auto buff = createRandomData<std::vector<unsigned char>>(1024 * 1024);
	for (auto _: state)
		crc32(0, buff.data(), buff.size());
}
static void GenericCRC32(benchmark::State& state)
{
	auto buff = createRandomData<std::vector<unsigned char>>(1024 * 1024);
	for (auto _: state)
		Crc32Impl::compute(0, buff.data(), buff.size());
}
static void GenericExCRC32(benchmark::State& state)
{
	auto buff = createRandomData<std::vector<unsigned char>>(1024 * 1024);
	uint64_t sum = 0;
	for (auto _: state) {
		auto result = Crc32Impl::compute_ex(0, buff.data(), buff.size());
		benchmark::DoNotOptimize(sum += result);
	}
}
static void GenericWide8CRC32(benchmark::State& state)
{
	auto buff = createRandomData<std::vector<unsigned char>>(1024 * 1024);
	uint64_t sum = 0;
	for (auto _: state) {
		auto result = Crc32Impl::compute_wide<8>(0, buff.data(), buff.size());
		benchmark::DoNotOptimize(sum += result);
	}
}
static void GenericWide16CRC32(benchmark::State& state)
{
	auto buff = createRandomData<std::vector<unsigned char>>(1024 * 1024);
	uint64_t sum = 0;
	for (auto _: state) {
		auto result = Crc32Impl::compute_wide<16>(0, buff.data(), buff.size());
		benchmark::DoNotOptimize(sum += result);
	}
}
static void GenericWide24CRC32(benchmark::State& state)
{
	auto buff = createRandomData<std::vector<unsigned char>>(1024 * 1024);
	uint64_t sum = 0;
	for (auto _: state) {
		auto result = Crc32Impl::compute_wide<24>(0, buff.data(), buff.size());
		benchmark::DoNotOptimize(sum += result);
	}
}

BENCHMARK(AdlerCRC32);
BENCHMARK(GenericCRC32);
BENCHMARK(GenericWide8CRC32);
BENCHMARK(GenericWide16CRC32);
BENCHMARK(GenericWide24CRC32);
BENCHMARK(GenericExCRC32);

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