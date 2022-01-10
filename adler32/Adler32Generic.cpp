#include "Adler32Generic.h"
#include <numeric>
#include <span>
#include <vector>

namespace Adler32::Generic {
static uint32_t compute1byte(uint32_t upper_component, uint32_t lower_component, unsigned char ch)
{
	lower_component += ch;
	upper_component += lower_component;
	upper_component %= constants::BASE;
	lower_component %= constants::BASE;
	return lower_component | (upper_component << 16);
}
static uint32_t compute16bytes(uint32_t upper_component, uint32_t lower_component, const unsigned char* buf, size_t len)
{
	for (uint32_t j = 0; j < len; ++j) {
		lower_component += buf[j];
		upper_component += lower_component;
	}
	lower_component %= constants::BASE;
	upper_component %= constants::BASE;
	return lower_component | (upper_component << 16);
}
uint32_t compute(uint32_t adler, const unsigned char* buf, size_t len)
{
	/* initial Adler-32 value (deferred check for len == 1 speed) */
	if (buf == nullptr)
		return 1;

	/* split Adler-32 into component sums */
	uint32_t upper_component = (adler >> 16) & 0xffff;
	uint32_t lower_component = adler & 0xffff;

	if (len == 1) {
		return compute1byte(upper_component, lower_component, buf[0]);
	} else if (len <= 16) {
		return compute16bytes(upper_component, lower_component, buf, len);
	}
	/* do length NMAX blocks -- requires just one modulo operation */

	/*
const auto blocks = (len + (constants::NMAX - 1)) / constants::NMAX;
for (size_t i = 0; i < blocks; ++i) {
	const auto block = i * constants::NMAX;
	for (uint32_t j = 0; j < constants::NMAX && block + j < len; ++j) {
		lower_componnet += buf[block + j];
		upper_component += lower_componnet;
	}
	lower_componnet %= constants::BASE;
	upper_component %= constants::BASE;
}
*/

	/* Four times slower
	 * for (size_t i = 0; i < len; ++i) {
		lower_componnet += buf[i];
		upper_component += lower_componnet;

		if ((i + 1) % constants::NMAX == 0 || i+1 == len) {
			lower_componnet %= constants::BASE;
			upper_component %= constants::BASE;
		}
	}*/

	/*
	 * less than 10% slower on 1MiB buffer
	 * twice slower for 1 KiB buffer
	 * */
	std::vector<uint32_t> components(constants::NMAX);
	size_t buff_pos = 0;
	std::span buffer_view(buf, len);
	for (auto it = buffer_view.begin(); it != buffer_view.end();) {
		const auto chunk_size = std::min(static_cast<size_t>(constants::NMAX), len - buff_pos);
		auto begin = it;
		std::advance(it, chunk_size);
		auto end = it;

		auto components_end =
				std::inclusive_scan(begin, end, components.begin(), std::plus<uint32_t>{}, lower_component);

		upper_component = std::accumulate(components.begin(), components_end, upper_component);
		upper_component %= constants::BASE;

		lower_component = *(--components_end);
		lower_component %= constants::BASE;

		buff_pos += chunk_size;
	}

	// return recombined sums
	return lower_component | (upper_component << 16);
}

uint32_t combine(uint32_t adler1, uint32_t adler2, off64_t len2)
{
	/* for negative len, return invalid adler32 as a clue for debugging */
	if (len2 < 0)
		return 0xffffffff;

	/* the derivation of this formula is left as an exercise for the reader */
	len2 %= constants::BASE; /* assumes len2 >= 0 */
	auto rem = static_cast<unsigned>(len2);
	uint32_t sum1 = adler1 & 0xffff;
	uint32_t sum2 = rem * sum1;
	sum2 %= constants::BASE;
	sum1 += (adler2 & 0xffff) + constants::BASE - 1;
	sum2 += ((adler1 >> 16) & 0xffff) + ((adler2 >> 16) & 0xffff) + constants::BASE - rem;
	if (sum1 >= constants::BASE)
		sum1 -= constants::BASE;
	if (sum1 >= constants::BASE)
		sum1 -= constants::BASE;
	if (sum2 >= (static_cast<unsigned long>(constants::BASE) << 1))
		sum2 -= (static_cast<unsigned long>(constants::BASE) << 1);
	if (sum2 >= constants::BASE)
		sum2 -= constants::BASE;
	return sum1 | (sum2 << 16);
}
}// namespace Adler32::Generic
