#define NONIUS_RUNNER
#include "Crc32.h"
#include "utils/BufferUtils.h"
#include <nonius.h++>
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
NONIUS_BENCHMARK("Compute original crc32 1MiB", [](nonius::chronometer cm) {
	auto buff = createRandomData<std::vector<unsigned char>>(1024 * 1024);
	cm.measure([&](int) { crc32(0, buff.data(), buff.size()); });
})

NONIUS_BENCHMARK("Compute crc32 1MiB", [](nonius::chronometer cm) {
	auto buff = createRandomData<std::vector<unsigned char>>(1024 * 1024);
	cm.measure([&](int) { Crc32Impl::compute(0, buff.data(), buff.size()); });
})

NONIUS_BENCHMARK("Compute crc32 x 4 bytes 1MiB", [](nonius::chronometer cm) {
	auto buff = createRandomData<std::vector<unsigned char>>(1024 * 1024);
	cm.measure([&](int foo) { Crc32Impl::compute_ex(0, buff.data(), buff.size()); });
})

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
