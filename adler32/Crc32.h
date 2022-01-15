#pragma once

#include <array>
#include <bit>
#include <cstdint>
#include <iostream>
#include <limits>

template<size_t Width>
struct unsigned_type {
	static_assert(Width > 0 && std::has_single_bit(Width) && Width < 9);
};

template<>
struct unsigned_type<1> {
	using type = unsigned char;
};
template<>
struct unsigned_type<2> {
	using type = uint16_t;
};
template<>
struct unsigned_type<4> {
	using type = uint32_t;
};
template<>
struct unsigned_type<8> {
	using type = uint64_t;
};
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

namespace CRC32::Generic {
uint32_t append_table(uint32_t crci, const unsigned char* input, size_t length);
uint32_t append_table_256(uint32_t crci, const unsigned char* input, size_t length);
uint32_t append_table_256_(uint32_t crci, const unsigned char* input, size_t length);
uint32_t append_table_256_1(uint32_t crci, const unsigned char* input, size_t length);
uint32_t append_table_256_2(uint32_t crci, const unsigned char* input, size_t length);
uint32_t append_table_256_3(uint32_t crci, const unsigned char* input, size_t length);

uint32_t append_table_4(uint32_t crc, const unsigned char* input, size_t length);
uint32_t append_table_8(uint32_t crc, const unsigned char* input, size_t length);
uint32_t append_table_16(uint32_t crc, const unsigned char* input, size_t length);
uint32_t append_table_24(uint32_t crc, const unsigned char* input, size_t length);
uint32_t append_table_32(uint32_t crc, const unsigned char* input, size_t length);


namespace {
template<size_t width>
constexpr uint32_t calcSomeBytes(uint32_t crc, auto some_bytes)
{
	crc ^= some_bytes;
	uint32_t accum = 0;
	for (size_t i = width; i > 0; i--) {
		accum ^= crc_tables[i - 1][(crc >> (std::numeric_limits<decltype(some_bytes)>::digits - i * 8)) & 0xff];
	}
	//	crc = crc_tables[3][crc & 0xff] ^ crc_tables[2][(crc >> 8) & 0xff] ^ crc_tables[1][(crc >> 16) & 0xff] ^
	//		  crc_tables[0][crc >> 24];
	return accum;
}

template<size_t width>
constexpr uint64_t calcBytes(uint64_t crc)
{
	return crc_tables[width - 1][(crc >> ((8 - width) * 8)) & 0xff] ^ calcBytes<width - 1>(crc);
}
template<>
constexpr uint64_t calcBytes<1>(uint64_t crc)
{
	return crc_tables[0][crc >> 56 & 0xff];
}

template<size_t width>
constexpr uint64_t calcInitialBytes(uint64_t crc, const uint8_t* buff)
{
	crc ^= *(reinterpret_cast<const uint64_t*>(buff));
	crc = calcBytes<8>(crc) ^ calcBytes<8>(*(reinterpret_cast<const uint64_t*>(buff + 8)));
	return crc;
}

constexpr uint32_t calc8Bytes(uint32_t crc, const uint8_t* buff)
{
	/*uint64_t working_crc = crc ^ eight_bytes;

	working_crc = crc_tables[7][working_crc & 0xff] ^ crc_tables[6][(working_crc >> 8) & 0xff] ^
				  crc_tables[5][(working_crc >> 16) & 0xff] ^ crc_tables[4][(working_crc >> 24) & 0xff] ^
				  crc_tables[3][(working_crc >> 32) & 0xff] ^ crc_tables[2][(working_crc >> 40) & 0xff] ^
				  crc_tables[1][(working_crc >> 48) & 0xff] ^ crc_tables[0][working_crc >> 56];
	return static_cast<uint32_t>(working_crc);*/
	return static_cast<uint32_t>(calcInitialBytes<8>(crc, buff));
}
constexpr uint32_t calc16Bytes(uint32_t crc, const uint8_t* buff)
{
	/*uint64_t working_crc = crc ^ eight_bytes;

	working_crc = crc_tables[7][working_crc & 0xff] ^ crc_tables[6][(working_crc >> 8) & 0xff] ^
				  crc_tables[5][(working_crc >> 16) & 0xff] ^ crc_tables[4][(working_crc >> 24) & 0xff] ^
				  crc_tables[3][(working_crc >> 32) & 0xff] ^ crc_tables[2][(working_crc >> 40) & 0xff] ^
				  crc_tables[1][(working_crc >> 48) & 0xff] ^ crc_tables[0][working_crc >> 56];
	return static_cast<uint32_t>(working_crc);*/
	return static_cast<uint32_t>(calcInitialBytes<8>(crc, buff));
}
}// namespace

uint32_t compute(uint32_t crc32, const unsigned char* data, size_t data_length);

template<size_t width>
uint32_t compute_wide(uint32_t crc, const unsigned char* buf, size_t len)
{
	crc = ~crc;
	auto four_byte_len = len / width;

	size_t buff_position = 0;
	for (; buff_position < four_byte_len; buff_position += width) {
		//		const auto* some_bytes = reinterpret_cast<const typename unsigned_type<width>::type*>(&buf[buff_position]);
		//		crc = calcSomeBytes<width>(crc, *some_bytes);
		//		crc = calc8Bytes(crc, *some_bytes);
		//		crc = calcBytes<8>(crc, &buf[buff_position]);
		crc = calc16Bytes(crc, &buf[buff_position]);
	}
	for (; buff_position < len; buff_position++) {
		crc = crc_tables[0][(crc ^ buf[buff_position]) & 0xff] ^ (crc >> 8);
	}
	crc = ~crc;
	return crc;
}

inline uint32_t compute_ex(uint32_t crc32, const unsigned char* data, size_t data_length)
{
	return compute_wide<register_width>(crc32, data, data_length);
}

}// namespace CRC32::Generic

namespace CRC32::SIMD {
uint32_t compute(uint32_t crc32, const unsigned char* data, size_t data_length);
}
