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
		/*std::cout << "lookup_index = " << static_cast<uint16_t>(lookup_index) << std::endl;
		std::cout << "init = " << init << std::endl;
		std::cout << "val = " << static_cast<uint16_t>(val) << std::endl;*/
		return (init >> 8) ^ crc_tables[0][lookup_index];
	});

	return crc32 ^ 0xFFFFFFFF;
}
uint32_t append_table(uint32_t crci, const unsigned char* input, size_t length)
{
	auto next = input;
	uint64_t crc;

	crc = crci ^ 0xffffffff;
	while (length && ((uintptr_t) next & 7) != 0) {
		crc = crc_tables[0][(crc ^ *next++) & 0xff] ^ (crc >> 8);
		--length;
	}
	while (length >= 16) {
		crc ^= *(const uint64_t*) next;
		uint64_t high = *(const uint64_t*) (next + 8);
		crc = crc_tables[15][crc & 0xff] ^ crc_tables[14][(crc >> 8) & 0xff] ^ crc_tables[13][(crc >> 16) & 0xff] ^
			  crc_tables[12][(crc >> 24) & 0xff] ^ crc_tables[11][(crc >> 32) & 0xff] ^
			  crc_tables[10][(crc >> 40) & 0xff] ^ crc_tables[9][(crc >> 48) & 0xff] ^ crc_tables[8][crc >> 56] ^
			  crc_tables[7][high & 0xff] ^ crc_tables[6][(high >> 8) & 0xff] ^ crc_tables[5][(high >> 16) & 0xff] ^
			  crc_tables[4][(high >> 24) & 0xff] ^ crc_tables[3][(high >> 32) & 0xff] ^
			  crc_tables[2][(high >> 40) & 0xff] ^ crc_tables[1][(high >> 48) & 0xff] ^ crc_tables[0][high >> 56];
		next += 16;
		length -= 16;
	}
	while (length) {
		crc = crc_tables[0][(crc ^ *next++) & 0xff] ^ (crc >> 8);
		--length;
	}
	return (uint32_t) crc ^ 0xffffffff;
}

uint32_t append_table_256(uint32_t crci, const unsigned char* input, size_t length)
{
	auto next = input;
	uint64_t crc;

	crc = crci ^ 0xffffffff;
//	while (length && ((uintptr_t) next & 7) != 0) {
//		crc = crc_tables[0][(crc ^ *next++) & 0xff] ^ (crc >> 8);
//		--length;
//	}
	while (length >= 32) {
		crc ^= *(const uint64_t*) next;
		uint64_t high1 = *(const uint64_t*) (next + 8);
		uint64_t high2 = *(const uint64_t*) (next + 16);
		uint64_t high3 = *(const uint64_t*) (next + 24);
		crc = crc_tables[31][crc & 0xff] ^
			  crc_tables[30][(crc >> 8) & 0xff] ^
			  crc_tables[29][(crc >> 16) & 0xff] ^
			  crc_tables[28][(crc >> 24) & 0xff] ^
			  crc_tables[27][(crc >> 32) & 0xff] ^
			  crc_tables[26][(crc >> 40) & 0xff] ^
			  crc_tables[25][(crc >> 48) & 0xff] ^
			  crc_tables[24][crc >> 56] ^
			  crc_tables[23][high1 & 0xff] ^
			  crc_tables[22][(high1 >> 8) & 0xff] ^
			  crc_tables[21][(high1 >> 16) & 0xff] ^
			  crc_tables[20][(high1 >> 24) & 0xff] ^
			  crc_tables[19][(high1 >> 32) & 0xff] ^
			  crc_tables[18][(high1 >> 40) & 0xff] ^
			  crc_tables[17][(high1 >> 48) & 0xff] ^
			  crc_tables[16][high1 >> 56] ^
			  crc_tables[15][high2 & 0xff] ^
			  crc_tables[14][(high2 >> 8) & 0xff] ^
			  crc_tables[13][(high2 >> 16) & 0xff] ^
			  crc_tables[12][(high2 >> 24) & 0xff] ^
			  crc_tables[11][(high2 >> 32) & 0xff] ^
			  crc_tables[10][(high2 >> 40) & 0xff] ^
			  crc_tables[9][(high2 >> 48) & 0xff] ^
			  crc_tables[8][high2 >> 56] ^
			  crc_tables[7][high3 & 0xff] ^
			  crc_tables[6][(high3 >> 8) & 0xff] ^
			  crc_tables[5][(high3 >> 16) & 0xff] ^
			  crc_tables[4][(high3 >> 24) & 0xff] ^
			  crc_tables[3][(high3 >> 32) & 0xff] ^
			  crc_tables[2][(high3 >> 40) & 0xff] ^
			  crc_tables[1][(high3 >> 48) & 0xff] ^
			  crc_tables[0][high3 >> 56];
		next += 32;
		length -= 32;
	}
	while (length) {
		crc = crc_tables[0][(crc ^ *next++) & 0xff] ^ (crc >> 8);
		--length;
	}
	return (uint32_t) crc ^ 0xffffffff;
}

template<size_t width>
constexpr uint32_t calcByte(const unsigned char* const buff)
{
	return crc_tables[width][*buff] ^ calcByte<width - 1>(buff + 1);
}
template<>
constexpr uint32_t calcByte<0>(const unsigned char* const buff)
{
	return crc_tables[0][*buff];
}

uint32_t append_table_256_(uint32_t crci, const unsigned char* input, size_t length)
{
	auto next = input;

	crci ^= 0xffffffff;
	while (length >= 32) {
		crci ^= *(const uint32_t*) next;
		crci = crc_tables[31][crci & 0xff] ^
			  crc_tables[30][(crci >> 8) & 0xff] ^
			  crc_tables[29][(crci >> 16) & 0xff] ^
			  crc_tables[28][(crci >> 24) & 0xff] ^
			  crc_tables[27][*(next+4)] ^
			  crc_tables[26][*(next+5)] ^
			  crc_tables[25][*(next+6)] ^
			  crc_tables[24][*(next+7)] ^
		/*crc ^= *next;
		crc = crc_tables[31][*(next)] ^
			  crc_tables[30][*(next+1)] ^
			  crc_tables[29][*(next+2)] ^
			  crc_tables[28][*(next+3)] ^
			  crc_tables[27][*(next+4)] ^
			  crc_tables[26][*(next+5)] ^
			  crc_tables[25][*(next+6)] ^
			  crc_tables[24][*(next+7)] ^*/
			  crc_tables[23][*(next+8)] ^
			  crc_tables[22][*(next+9)] ^
			  crc_tables[21][*(next+10)] ^
			  crc_tables[20][*(next+11)] ^
			  crc_tables[19][*(next+12)] ^
			  crc_tables[18][*(next+13)] ^
			  crc_tables[17][*(next+14)] ^
			  crc_tables[16][*(next+15)] ^
			  crc_tables[15][*(next+16)] ^
			  crc_tables[14][*(next+17)] ^
			  crc_tables[13][*(next+18)] ^
			  crc_tables[12][*(next+19)] ^
			  crc_tables[11][*(next+20)] ^
			  crc_tables[10][*(next+21)] ^
			  crc_tables[9][*(next+22)] ^
			  crc_tables[8][*(next+23)] ^
			  crc_tables[7][*(next+24)] ^
			  crc_tables[6][*(next+25)] ^
			  crc_tables[5][*(next+26)] ^
			  crc_tables[4][*(next+27)] ^
			  crc_tables[3][*(next+28)] ^
			  crc_tables[2][*(next+29)] ^
			  crc_tables[1][*(next+30)] ^
			  crc_tables[0][*(next+31)];
		next += 32;
		length -= 32;
	}
	while (length) {
		crci = crc_tables[0][(crci ^ *next++) & 0xff] ^ (crci >> 8);
		--length;
	}
	return crci ^ 0xffffffff;
}

uint32_t append_table_256_1(uint32_t crci, const unsigned char* input, size_t length)
{
	auto next = input;

	crci ^= 0xffffffff;
	while (length >= 32) {
		crci ^= *(const uint32_t*) next;
		crci = crc_tables[31][crci & 0xff] ^
			   crc_tables[30][(crci >> 8) & 0xff] ^
			   crc_tables[29][(crci >> 16) & 0xff] ^
			   crc_tables[28][(crci >> 24) & 0xff] ^
			   calcByte<27>(next + 4);
		next += 32;
		length -= 32;
	}
	while (length) {
		crci = crc_tables[0][(crci ^ *next++) & 0xff] ^ (crci >> 8);
		--length;
	}
	return crci ^ 0xffffffff;
}

uint32_t append_table_256_2(uint32_t crc, const unsigned char* input, size_t length)
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

uint32_t append_table_256_3(uint32_t crc, const unsigned char* input, size_t length)
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

template <unsigned width>
uint32_t append_table_tmpl(uint32_t crc, const unsigned char* input, size_t length)
{
	crc = ~crc;

	while (length > width) {
		crc ^= *(reinterpret_cast<const uint32_t*>(input));
		crc = crc_tables[width-1][crc & 0xff] ^ crc_tables[width-2][(crc >> 8) & 0xff] ^ crc_tables[width-3][(crc >> 16) & 0xff] ^
			  crc_tables[width-4][(crc >> 24) & 0xff] ^ calcByte<width-5>(input + 4);
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
uint32_t append_table_4(uint32_t crc, const unsigned char* input, size_t length)
{
	return append_table_tmpl<8>(crc, input, length);
}
uint32_t append_table_8(uint32_t crc, const unsigned char* input, size_t length)
{
	return append_table_tmpl<8>(crc, input, length);
}
uint32_t append_table_16(uint32_t crc, const unsigned char* input, size_t length)
{
	return append_table_tmpl<16>(crc, input, length);
}
uint32_t append_table_24(uint32_t crc, const unsigned char* input, size_t length)
{
	return append_table_tmpl<24>(crc, input, length);
}
uint32_t append_table_32(uint32_t crc, const unsigned char* input, size_t length)
{
	return append_table_tmpl<32>(crc, input, length);
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
