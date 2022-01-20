#include "Crc32.h"
#include <numeric>
#include <span>

namespace CRC32::Generic {
uint32_t compute(uint32_t crc32, const unsigned char* data, size_t data_length)
{
	crc32 ^= 0xFFFFFFFF;

	std::span buffer_view(data, data_length);
	crc32 = std::accumulate(buffer_view.begin(), buffer_view.end(), crc32, [](auto init, auto val) {
		const uint32_t lookup_index = (init ^ val) & 0xff;
		return (init >> 8) ^ crc_tables[0][lookup_index];
	});

	return crc32 ^ 0xFFFFFFFF;
}

}// namespace CRC32::Generic
