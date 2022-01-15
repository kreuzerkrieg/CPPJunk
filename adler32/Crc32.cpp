#include "Crc32.h"
#include <eve/algo/for_each.hpp>
#include <eve/function/scan.hpp>
#include <eve/views/convert.hpp>
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

uint32_t compute_256_1(uint32_t crc, const unsigned char* input, size_t length)
{
	crc = ~crc;
	auto reg_len = length / register_width;

	size_t buff_position = 0;
	for (; buff_position < reg_len; buff_position += register_width) {
		crc ^= *(reinterpret_cast<const uint32_t*>(&input[buff_position]));
		crc = crc_tables[31][crc & 0xff] ^ crc_tables[30][(crc >> 8) & 0xff] ^ crc_tables[29][(crc >> 16) & 0xff] ^
			  crc_tables[28][(crc >> 24) & 0xff] ^ calcByte<27>(&input[buff_position + 4]);
	}

	for (; buff_position < length; buff_position++) {
		crc = crc_tables[0][(crc ^ input[buff_position]) & 0xff] ^ (crc >> 8);
	}
	crc = ~crc;
	return crc;
}

uint32_t compute_256_2(uint32_t crc, const unsigned char* input, size_t length)
{
	crc = ~crc;

	while (length > register_width) {
		crc ^= *(reinterpret_cast<const uint32_t*>(input));
		crc = crc_tables[31][crc & 0xff] ^ crc_tables[30][(crc >> 8) & 0xff] ^ crc_tables[29][(crc >> 16) & 0xff] ^
			  crc_tables[28][(crc >> 24) & 0xff] ^ calcByte<27>(input + 4);
		input += register_width;
		length -= register_width;
	}

	while (length > 0) {
		crc = crc_tables[0][(crc ^ *input) & 0xff] ^ (crc >> 8);
		++input;
		--length;
	}
	crc = ~crc;
	return crc;
}

}// namespace CRC32::Generic

namespace CRC32::SIMD {
uint32_t compute(uint32_t crc32, const unsigned char* data, size_t data_length)
{
	eve::wide<uint32_t> retval{crc32 ^ 0xFFFFFFFF};
	constexpr std::uint32_t zero{0};

	std::span buffer_view(data, data_length);
	auto in_as_uint32 = eve::views::convert(buffer_view, eve::as<std::uint32_t>{});
	//	std::span table(CRCTable);
	//	auto wide_crc_table = eve::views::convert(table, eve::as<std::uint32_t>{});

	eve::algo::for_each(in_as_uint32, [&](auto iter, auto ignore) {
		eve::wide<std::uint32_t> chunk = eve::load[ignore.else_(zero)](iter);

		/*retval = eve::scan(
				chunk,
				[&retval](auto it, auto skip) {
					// eve::wide<std::uint32_t> byte = eve::read(it);
					// eve::wide<std::uint32_t> b = eve::load[eve::ignore_all(zero)](it);
					std::uint32_t byte = it.front();
					auto lookup_index = ((retval ^ byte) & 0xff).front();
					return (retval >> 8) ^ CRCTable[lookup_index];
				},
				chunk);*/
		retval = eve::scan(chunk);
		/*auto _1 = lookup_index.get(0);
		auto _2 = lookup_index.get(1);
		auto _4 = lookup_index.get(2);
		auto _5 = lookup_index.get(3);
		using table_values = eve::wide<std::uint32_t, eve::fixed<4>>;
		table_values values = {CRCTable[lookup_index.get(0)], CRCTable[lookup_index.get(1)],
							   CRCTable[lookup_index.get(2)], CRCTable[lookup_index.get(3)]};
		retval = (retval >> 8) ^ values;*/
	});

	retval ^= 0xFFFFFFFF;
	auto a = retval.front();
	return a;
}
}// namespace CRC32::SIMD
