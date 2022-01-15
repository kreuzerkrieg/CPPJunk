#pragma once

#include "Crc32_constans.h"

namespace CRC32::Generic {

uint32_t compute(uint32_t crc32, const unsigned char* data, size_t data_length);
uint32_t compute_256_1(uint32_t crc32, const unsigned char* data, size_t data_length);
uint32_t compute_256_2(uint32_t crc32, const unsigned char* data, size_t data_length);

template<uint8_t width>
constexpr uint32_t calcByte(const unsigned char* const buff)
{
	return crc_tables[width][*buff] ^ calcByte<width - 1>(buff + 1);
}

template<>
constexpr uint32_t calcByte<0>(const unsigned char* const buff)
{
	return crc_tables[0][*buff];
}

template<uint8_t width>
uint32_t compute_wide(uint32_t crc, const unsigned char* input, size_t length)
{
	crc = ~crc;

	while (length > width) {
		crc ^= *(reinterpret_cast<const uint32_t*>(input));
		crc = crc_tables[width - 1][crc & 0xff] ^ crc_tables[width - 2][(crc >> 8) & 0xff] ^
			  crc_tables[width - 3][(crc >> 16) & 0xff] ^ crc_tables[width - 4][(crc >> 24) & 0xff] ^
			  calcByte<width - 5>(input + 4);
		input += width;
		length -= width;
	}

	while (length > 0) {
		crc = crc_tables[0][(crc ^ *input) & 0xff] ^ (crc >> 8);
		++input;
		--length;
	}
	crc = ~crc;
	return crc;
}

/*template<>
uint32_t compute_wide<4>(uint32_t crc, const unsigned char* input, size_t length)
{
	uint16_t width = 4;
	crc = ~crc;
	while (length > width) {
		crc ^= *(reinterpret_cast<const uint32_t*>(input));
		crc = crc_tables[width - 1][crc & 0xff] ^ crc_tables[width - 2][(crc >> 8) & 0xff] ^
			  crc_tables[width - 3][(crc >> 16) & 0xff] ^ crc_tables[width - 4][(crc >> 24) & 0xff];
		input += width;
		length -= width;
	}

	while (length > 0) {
		crc = crc_tables[0][(crc ^ *input) & 0xff] ^ (crc >> 8);
		++input;
		--length;
	}
	crc = ~crc;
	return crc;
}*/

inline uint32_t compute_ex(uint32_t crc32, const unsigned char* data, size_t data_length)
{
	return compute_wide<register_width>(crc32, data, data_length);
}

}// namespace CRC32::Generic

namespace CRC32::SIMD {
uint32_t compute(uint32_t crc32, const unsigned char* data, size_t data_length);
}
