#define NONIUS_RUNNER
#include "Adler32.h"
#include "utils/BufferUtils.h"
#include <nonius.h++>
#include <zlib.h>

NONIUS_BENCHMARK("Compute original Adler32 1MiB", [](nonius::chronometer cm) {
	auto buff = createRandomData<std::vector<unsigned char>>(1024 * 1024);
	cm.measure([&](int) { adler32(0, buff.data(), buff.size()); });
})

NONIUS_BENCHMARK("Compute Adler32 1MiB", [](nonius::chronometer cm) {
	auto buff = createRandomData<std::vector<unsigned char>>(1024 * 1024);
	cm.measure([&](int) { Adler32::compute(0, buff.data(), buff.size()); });
})