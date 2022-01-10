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
