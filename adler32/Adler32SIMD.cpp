#include "Adler32Generic.h"
#include <eve/algo/for_each.hpp>
#include <eve/views/convert.hpp>
#include <span>

#include <eve/function/add.hpp>
#include <eve/function/load.hpp>
#include <eve/function/reduce.hpp>
#include <eve/function/scan.hpp>

namespace Adler32::SIMD {

uint32_t compute(uint32_t adler, const unsigned char* buf, size_t len)
{
	/* initial Adler-32 value (deferred check for len == 1 speed) */
	if (buf == nullptr)
		return 1;

	/* split Adler-32 into component sums */
	eve::wide<std::uint32_t> upper_component{(adler >> 16) & 0xffff};
	eve::wide<std::uint32_t> lower_component{adler & 0xffff};

	const std::uint32_t zero{0};
	size_t buff_pos = 0;
	std::span buffer_view(buf, len);
	for (auto it = buffer_view.begin(); it != buffer_view.end();) {
		const auto chunk_size = std::min(static_cast<size_t>(constants::NMAX), len - buff_pos);
		auto subspan = buffer_view.subspan(buff_pos, chunk_size);
		auto in_as_uint32 = eve::views::convert(subspan, eve::as<std::uint32_t>{});

		eve::wide<std::uint32_t> running_sum{lower_component};
		eve::wide<std::uint32_t> sum_of_sums{zero};
		eve::algo::for_each(in_as_uint32, [&](auto iter, auto ignore) {
			// load the value. if some elements should be ignored, replace them with zero
			eve::wide<std::uint32_t> xs = eve::load[ignore.else_(zero)](iter);

			xs = eve::scan(xs);
			xs += running_sum;

			// add[ignore] will keep the previous sum_of_sums for ignored elements
			sum_of_sums = eve::add[ignore](sum_of_sums, xs);

			running_sum = xs.back();
		});

		lower_component = running_sum.back();
		upper_component += eve::reduce(sum_of_sums);

		lower_component %= constants::BASE;
		upper_component %= constants::BASE;

		std::advance(it, chunk_size);
		buff_pos += chunk_size;
	}

	// return recombined sums
	return lower_component.front() | (upper_component.front() << 16);
}

uint32_t combine(uint32_t adler1, uint32_t adler2, off64_t len2)
{
	return Adler32::Generic::combine(adler1, adler2, len2);
}
}// namespace Adler32::SIMD
