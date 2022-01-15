#pragma once

#include <array>
#include <bit>
#include <cstdint>

inline constexpr uint32_t ZSWAP32(uint32_t q)
{
	return ((((q) >> 24) & 0xff) + (((q) >> 8) & 0xff00) + (((q) &0xff00) << 8) + (((q) &0xff) << 24));
}

constexpr uint16_t register_width_bits = 256;
constexpr uint16_t register_width = register_width_bits / 8;
constexpr std::array<std::array<uint32_t, 256>, register_width* 2> crc_tables = []() constexpr
{
	std::array<std::array<uint32_t, 256>, register_width* 2> retval = {};

	/* terms of polynomial defining this crc (except x^32): */
	constexpr std::array<unsigned char, 14> p = {0, 1, 2, 4, 5, 7, 8, 10, 11, 12, 16, 22, 23, 26};

	/* make exclusive-or pattern from polynomial (0xedb88320UL) */
	uint32_t poly = 0; /* polynomial exclusive-or pattern */
	for (auto bit: p)
		poly |= static_cast<uint32_t>(1) << (31 - bit);

	/* generate a crc for every 8-bit value */
	for (uint32_t n = 0; n < 256; ++n) {
		auto c = n;
		for (int k = 0; k < 8; ++k) {
			c = c & 1 ? poly ^ (c >> 1) : c >> 1;
		}
		retval[0][n] = c;
	}

	/* generate crc for each value followed by one, two, and three zeros,
           and then the byte reversal of those as well as the first table */
	if constexpr (register_width > 1) {
		for (uint32_t n = 0; n < 256; ++n) {
			auto c = retval[0][n];
			retval[register_width][n] = ZSWAP32(c);
			for (uint32_t k = 1; k < register_width; k++) {
				c = retval[0][c & 0xff] ^ (c >> 8);
				retval[k][n] = c;
				retval[k + register_width][n] = ZSWAP32(c);
			}
		}
	}
	return retval;
}
();
