#pragma once

#include "Crc32_constans.h"

namespace CRC32::Generic {

uint32_t compute(uint32_t crc32, const unsigned char* data, size_t data_length);

template<uint8_t width>
constexpr uint32_t calcByte(const unsigned char* const buff) noexcept
{
	return crc_tables[width][*buff] ^ calcByte<width - 1>(buff + 1);
}

template<>
constexpr uint32_t calcByte<0>(const unsigned char* const buff) noexcept
{
	return crc_tables[0][*buff];
}

template<uint8_t width>
constexpr uint32_t compute_wide(uint32_t crc, const unsigned char* input, size_t length) noexcept
{
	const auto reg_len = length / width;
	size_t buff_position = 0;

	crc = ~crc;
	for (size_t i = 0; i < reg_len; i++) {
		crc ^= *(reinterpret_cast<const uint32_t*>(&input[buff_position]));
		crc = crc_tables[width - 1][crc & 0xff] ^ crc_tables[width - 2][(crc >> 8) & 0xff] ^
			  crc_tables[width - 3][(crc >> 16) & 0xff] ^ crc_tables[width - 4][(crc >> 24) & 0xff] ^
			  calcByte<width - 5>(&input[buff_position] + 4);
		buff_position += width;
	}

	for (; buff_position < length; buff_position++) {
		crc = crc_tables[0][(crc ^ input[buff_position]) & 0xff] ^ (crc >> 8);
	}
	return ~crc;
}

template<>
constexpr uint32_t compute_wide<4>(uint32_t crc, const unsigned char* input, size_t length) noexcept
{
	constexpr uint16_t width = 4;
	const auto reg_len = length / width;
	size_t buff_position = 0;

	crc = ~crc;
	for (size_t i = 0; i < reg_len; i++) {
		crc ^= *(reinterpret_cast<const uint32_t*>(&input[buff_position]));
		crc = crc_tables[width - 1][crc & 0xff] ^ crc_tables[width - 2][(crc >> 8) & 0xff] ^
			  crc_tables[width - 3][(crc >> 16) & 0xff] ^ crc_tables[width - 4][(crc >> 24) & 0xff];
		buff_position += width;
	}

	for (; buff_position < length; buff_position++) {
		crc = crc_tables[0][(crc ^ input[buff_position]) & 0xff] ^ (crc >> 8);
	}
	return ~crc;
}

constexpr uint32_t compute_ex(uint32_t crc32, const unsigned char* data, size_t data_length) noexcept
{
	return compute_wide<register_width>(crc32, data, data_length);
}

}// namespace CRC32::Generic

namespace CRC32::Intrinsic {
uint32_t compute(uint32_t crc32, const unsigned char* data, size_t data_length) noexcept;
}
