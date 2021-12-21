#include "Adler32.h"

uint32_t Adler32::compute(uint32_t adler, const unsigned char* buf, size_t len)
{
	/* initial Adler-32 value (deferred check for len == 1 speed) */
	if (buf == nullptr)
		return 1;

	/* split Adler-32 into component sums */
	uint32_t sum2 = (adler >> 16) & 0xffff;
	adler &= 0xffff;

	/* do length NMAX blocks -- requires just one modulo operation */

	const auto blocks = (len + (constants::NMAX - 1)) / constants::NMAX;
	size_t buff_pos = 0;
	for (size_t i = 0; i < blocks; ++i) {
		for (uint32_t j = 0; j < constants::NMAX && buff_pos < len; ++j, ++buff_pos) {
			adler += buf[buff_pos];
			sum2 += adler;
		}
		adler %= constants::BASE;
		sum2 %= constants::BASE;
	}

	/* return recombined sums */
	return adler | (sum2 << 16);
}

uint32_t Adler32::combine(uint32_t adler1, uint32_t adler2, off64_t len2)
{
	uint32_t sum1;
	uint32_t sum2;
	unsigned rem;

	/* for negative len, return invalid adler32 as a clue for debugging */
	if (len2 < 0)
		return 0xffffffff;

	/* the derivation of this formula is left as an exercise for the reader */
	len2 %= constants::BASE; /* assumes len2 >= 0 */
	rem = static_cast<unsigned>(len2);
	sum1 = adler1 & 0xffff;
	sum2 = rem * sum1;
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
