#define NONIUS_RUNNER
#include "Adler32Generic.h"
#include "utils/BufferUtils.h"
#include <nonius.h++>
#include <zlib.h>
namespace Adler32Impl = Adler32::SIMD;

NONIUS_BENCHMARK("Compute original Adler32 1KiB", [](nonius::chronometer cm) {
	auto buff = createRandomData<std::vector<unsigned char>>(1024);
	cm.measure([&](int) { adler32(0, buff.data(), buff.size()); });
})

NONIUS_BENCHMARK("Compute Adler32 1KiB", [](nonius::chronometer cm) {
	auto buff = createRandomData<std::vector<unsigned char>>(1024);
	cm.measure([&](int) { Adler32Impl::compute(0, buff.data(), buff.size()); });
})

NONIUS_BENCHMARK("Compute original Adler32 1MiB", [](nonius::chronometer cm) {
	auto buff = createRandomData<std::vector<unsigned char>>(1024 * 1024);
	cm.measure([&](int) { adler32(0, buff.data(), buff.size()); });
})

NONIUS_BENCHMARK("Compute Adler32 1MiB", [](nonius::chronometer cm) {
	auto buff = createRandomData<std::vector<unsigned char>>(1024 * 1024);
	cm.measure([&](int) { Adler32Impl::compute(0, buff.data(), buff.size()); });
})

NONIUS_BENCHMARK("Compute original Adler32 1 byte", [](nonius::chronometer cm) {
	auto buff = createRandomData<std::vector<unsigned char>>(1);
	cm.measure([&](int) { adler32(0, buff.data(), buff.size()); });
})

NONIUS_BENCHMARK("Compute Adler32 1 byte", [](nonius::chronometer cm) {
	auto buff = createRandomData<std::vector<unsigned char>>(1);
	cm.measure([&](int) { Adler32Impl::compute(0, buff.data(), buff.size()); });
})

NONIUS_BENCHMARK("Compute original Adler32 15 bytes", [](nonius::chronometer cm) {
	auto buff = createRandomData<std::vector<unsigned char>>(15);
	cm.measure([&](int) { adler32(0, buff.data(), buff.size()); });
})

NONIUS_BENCHMARK("Compute Adler32 15 bytes", [](nonius::chronometer cm) {
	auto buff = createRandomData<std::vector<unsigned char>>(15);
	cm.measure([&](int) { Adler32Impl::compute(0, buff.data(), buff.size()); });
})

NONIUS_BENCHMARK("Combine original Adler32 1MiB", [](nonius::chronometer cm) {
	auto buff = createRandomData<std::vector<unsigned char>>(1024 * 1024);
	uint32_t adler = adler32(0, buff.data(), buff.size());
	cm.measure([&](int) {
		for (int i = 0; i < 3; ++i) {
			adler = adler32_combine(adler, adler + i, buff.size());
		}
	});
})

NONIUS_BENCHMARK("Combine Adler32 1MiB", [](nonius::chronometer cm) {
	auto buff = createRandomData<std::vector<unsigned char>>(1024 * 1024);
	uint32_t adler32 = Adler32Impl::compute(adler32, buff.data(), buff.size());
	cm.measure([&](int) {
		for (int i = 0; i < 3; ++i) {
			adler32 = Adler32Impl::combine(adler32, adler32 + i, buff.size());
		}
	});
})
