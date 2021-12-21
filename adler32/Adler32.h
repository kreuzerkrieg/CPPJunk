#pragma once

#include <cstdio>
#include <string_view>
namespace constants {
static constexpr uint32_t BASE = 65521U; /* largest prime smaller than 65536 */
static constexpr uint32_t NMAX = 5552;   /* NMAX is the largest n such that 255n(n+1)/2 + (n+1)(BASE-1) <= 2^32-1 */
}// namespace constants

class Adler32
{
	using buffer_view = std::basic_string_view<unsigned char>;

public:
	static uint32_t compute(uint32_t adler, const unsigned char* buf, size_t len);
	static uint32_t combine(uint32_t adler1, uint32_t adler2, off64_t len2);
};
