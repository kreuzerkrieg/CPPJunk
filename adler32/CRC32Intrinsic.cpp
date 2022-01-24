/*
* Compute the CRC32 using a parallelized folding approach with the PCLMULQDQ
* instruction.
*
* A white paper describing this algorithm can be found at:
* http://www.intel.com/content/dam/www/public/us/en/documents/white-papers/fast-crc-computation-generic-polynomials-pclmulqdq-paper.pdf
*
* Copyright (C) 2013 Intel Corporation. All rights reserved.
* Authors:
* 	Wajdi Feghali   <wajdi.k.feghali@intel.com>
* 	Jim Guilford    <james.guilford@intel.com>
* 	Vinodh Gopal    <vinodh.gopal@intel.com>
* 	Erdinc Ozturk   <erdinc.ozturk@intel.com>
* 	Jim Kukunas     <james.t.kukunas@linux.intel.com>
*
* For conditions of distribution and use, see copyright notice in zlib.h
*/

#include "Crc32.h"
#include <array>
#include <vector>
#include <x86intrin.h>

namespace CRC32::Intrinsic {
struct alignas(64) crc512 {
	std::array<__m128i, 4> crcs{0x9db42487, 0, 0, 0};
};

//static std::array<__m128i, 4> crc512{0x9db42487, 0, 0, 0};

static const __m128i xmm_fold4 = _mm_set_epi32(0x00000001, 0x54442bd4, 0x00000001, static_cast<int>(0xc6e41596));
static const __m128i xmm_mask3 = _mm_set1_epi32(static_cast<int>(0x80808080));

static void fold_1(__m128i& xmm_crc0, __m128i& xmm_crc1, __m128i& xmm_crc2, __m128i& xmm_crc3)
{
	auto tmp_crc1 = xmm_crc1;
	xmm_crc1 = xmm_crc2;
	xmm_crc2 = xmm_crc3;

	xmm_crc3 = _mm_clmulepi64_si128(xmm_crc0, xmm_fold4, 0x10);
	xmm_crc0 = _mm_clmulepi64_si128(xmm_crc0, xmm_fold4, 0x01);
	auto ps_res = _mm_xor_ps(_mm_castsi128_ps(xmm_crc0), _mm_castsi128_ps(xmm_crc3));
	xmm_crc3 = _mm_castps_si128(ps_res);

	xmm_crc0 = tmp_crc1;
}

static void fold_2(__m128i& xmm_crc0, __m128i& xmm_crc1, __m128i& xmm_crc2, __m128i& xmm_crc3)
{
	auto x_tmp3 = xmm_crc3;
	auto x_tmp2 = xmm_crc2;

	xmm_crc3 = _mm_clmulepi64_si128(xmm_crc1, xmm_fold4, 0x10);
	xmm_crc1 = _mm_clmulepi64_si128(xmm_crc1, xmm_fold4, 0x01);
	auto ps_res31 = _mm_xor_ps(_mm_castsi128_ps(xmm_crc3), _mm_castsi128_ps(xmm_crc1));
	xmm_crc3 = _mm_castps_si128(ps_res31);

	xmm_crc2 = _mm_clmulepi64_si128(xmm_crc0, xmm_fold4, 0x10);
	xmm_crc0 = _mm_clmulepi64_si128(xmm_crc0, xmm_fold4, 0x01);
	auto ps_res20 = _mm_xor_ps(_mm_castsi128_ps(xmm_crc0), _mm_castsi128_ps(xmm_crc2));
	xmm_crc2 = _mm_castps_si128(ps_res20);

	xmm_crc0 = x_tmp2;
	xmm_crc1 = x_tmp3;
}

static void fold_3(__m128i& xmm_crc0, __m128i& xmm_crc1, __m128i& xmm_crc2, __m128i& xmm_crc3)
{
	auto x_tmp3 = xmm_crc3;

	xmm_crc3 = _mm_clmulepi64_si128(xmm_crc2, xmm_fold4, 0x10);
	xmm_crc2 = _mm_clmulepi64_si128(xmm_crc2, xmm_fold4, 0x01);
	auto ps_res32 = _mm_xor_ps(_mm_castsi128_ps(xmm_crc2), _mm_castsi128_ps(xmm_crc3));
	xmm_crc3 = _mm_castps_si128(ps_res32);

	xmm_crc2 = _mm_clmulepi64_si128(xmm_crc1, xmm_fold4, 0x10);
	xmm_crc1 = _mm_clmulepi64_si128(xmm_crc1, xmm_fold4, 0x01);
	auto ps_res21 = _mm_xor_ps(_mm_castsi128_ps(xmm_crc1), _mm_castsi128_ps(xmm_crc2));
	xmm_crc2 = _mm_castps_si128(ps_res21);

	xmm_crc1 = _mm_clmulepi64_si128(xmm_crc0, xmm_fold4, 0x10);
	xmm_crc0 = _mm_clmulepi64_si128(xmm_crc0, xmm_fold4, 0x01);
	auto ps_res10 = _mm_xor_ps(_mm_castsi128_ps(xmm_crc0), _mm_castsi128_ps(xmm_crc1));
	xmm_crc1 = _mm_castps_si128(ps_res10);

	xmm_crc0 = x_tmp3;
}

static void fold_4(crc512& crc)
{

	auto x_tmp0 = _mm_clmulepi64_si128(crc.crcs[0], xmm_fold4, 0x10);
	crc.crcs[0] = _mm_clmulepi64_si128(crc.crcs[0], xmm_fold4, 0x01);
	auto ps_res0 = _mm_xor_ps(_mm_castsi128_ps(crc.crcs[0]), _mm_castsi128_ps(x_tmp0));
	crc.crcs[0] = _mm_castps_si128(ps_res0);

	auto x_tmp1 = _mm_clmulepi64_si128(crc.crcs[1], xmm_fold4, 0x10);
	crc.crcs[1] = _mm_clmulepi64_si128(crc.crcs[1], xmm_fold4, 0x01);
	auto ps_res1 = _mm_xor_ps(_mm_castsi128_ps(crc.crcs[1]), _mm_castsi128_ps(x_tmp1));
	crc.crcs[1] = _mm_castps_si128(ps_res1);

	auto x_tmp2 = _mm_clmulepi64_si128(crc.crcs[2], xmm_fold4, 0x10);
	crc.crcs[2] = _mm_clmulepi64_si128(crc.crcs[2], xmm_fold4, 0x01);
	auto ps_res2 = _mm_xor_ps(_mm_castsi128_ps(crc.crcs[2]), _mm_castsi128_ps(x_tmp2));
	crc.crcs[2] = _mm_castps_si128(ps_res2);

	auto x_tmp3 = _mm_clmulepi64_si128(crc.crcs[3], xmm_fold4, 0x10);
	crc.crcs[3] = _mm_clmulepi64_si128(crc.crcs[3], xmm_fold4, 0x01);
	auto ps_res3 = _mm_xor_ps(_mm_castsi128_ps(crc.crcs[3]), _mm_castsi128_ps(x_tmp3));
	crc.crcs[3] = _mm_castps_si128(ps_res3);
}

constexpr unsigned __attribute__((aligned(32))) pshufb_shf_table[60] = {
		0x84838281, 0x88878685, 0x8c8b8a89, 0x008f8e8d, /* shl 15 (16 - 1)/shr1 */
		0x85848382, 0x89888786, 0x8d8c8b8a, 0x01008f8e, /* shl 14 (16 - 3)/shr2 */
		0x86858483, 0x8a898887, 0x8e8d8c8b, 0x0201008f, /* shl 13 (16 - 4)/shr3 */
		0x87868584, 0x8b8a8988, 0x8f8e8d8c, 0x03020100, /* shl 12 (16 - 4)/shr4 */
		0x88878685, 0x8c8b8a89, 0x008f8e8d, 0x04030201, /* shl 11 (16 - 5)/shr5 */
		0x89888786, 0x8d8c8b8a, 0x01008f8e, 0x05040302, /* shl 10 (16 - 6)/shr6 */
		0x8a898887, 0x8e8d8c8b, 0x0201008f, 0x06050403, /* shl  9 (16 - 7)/shr7 */
		0x8b8a8988, 0x8f8e8d8c, 0x03020100, 0x07060504, /* shl  8 (16 - 8)/shr8 */
		0x8c8b8a89, 0x008f8e8d, 0x04030201, 0x08070605, /* shl  7 (16 - 9)/shr9 */
		0x8d8c8b8a, 0x01008f8e, 0x05040302, 0x09080706, /* shl  6 (16 -10)/shr10*/
		0x8e8d8c8b, 0x0201008f, 0x06050403, 0x0a090807, /* shl  5 (16 -11)/shr11*/
		0x8f8e8d8c, 0x03020100, 0x07060504, 0x0b0a0908, /* shl  4 (16 -12)/shr12*/
		0x008f8e8d, 0x04030201, 0x08070605, 0x0c0b0a09, /* shl  3 (16 -13)/shr13*/
		0x01008f8e, 0x05040302, 0x09080706, 0x0d0c0b0a, /* shl  2 (16 -14)/shr14*/
		0x0201008f, 0x06050403, 0x0a090807, 0x0e0d0c0b  /* shl  1 (16 -15)/shr15*/
};

static void partial_fold(size_t len, __m128i& xmm_crc0, __m128i& xmm_crc1, __m128i& xmm_crc2, __m128i& xmm_crc3,
						 __m128i& xmm_crc_part)
{
	auto xmm_shl = _mm_load_si128(reinterpret_cast<const __m128i*>(pshufb_shf_table) + (len - 1));
	auto xmm_shr = xmm_shl;
	xmm_shr = _mm_xor_si128(xmm_shr, xmm_mask3);

	auto xmm_a0_0 = _mm_shuffle_epi8(xmm_crc0, xmm_shl);

	xmm_crc0 = _mm_shuffle_epi8(xmm_crc0, xmm_shr);
	auto xmm_tmp1 = _mm_shuffle_epi8(xmm_crc1, xmm_shl);
	xmm_crc0 = _mm_or_si128(xmm_crc0, xmm_tmp1);

	xmm_crc1 = _mm_shuffle_epi8(xmm_crc1, xmm_shr);
	auto xmm_tmp2 = _mm_shuffle_epi8(xmm_crc2, xmm_shl);
	xmm_crc1 = _mm_or_si128(xmm_crc1, xmm_tmp2);

	xmm_crc2 = _mm_shuffle_epi8(xmm_crc2, xmm_shr);
	auto xmm_tmp3 = _mm_shuffle_epi8(xmm_crc3, xmm_shl);
	xmm_crc2 = _mm_or_si128(xmm_crc2, xmm_tmp3);

	xmm_crc3 = _mm_shuffle_epi8(xmm_crc3, xmm_shr);
	xmm_crc_part = _mm_shuffle_epi8(xmm_crc_part, xmm_shl);
	xmm_crc3 = _mm_or_si128(xmm_crc3, xmm_crc_part);

	auto xmm_a0_1 = _mm_clmulepi64_si128(xmm_a0_0, xmm_fold4, 0x10);
	xmm_a0_0 = _mm_clmulepi64_si128(xmm_a0_0, xmm_fold4, 0x01);

	auto ps_res = _mm_xor_ps(_mm_castsi128_ps(xmm_crc3), _mm_castsi128_ps(xmm_a0_0));
	ps_res = _mm_xor_ps(ps_res, _mm_castsi128_ps(xmm_a0_1));

	xmm_crc3 = _mm_castps_si128(ps_res);
}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-braces"
constexpr std::array<uint32_t, 12> crc_k[] = {
		0xccaa009e, 0x00000000, /* rk1 */
		0x751997d0, 0x00000001, /* rk2 */
		0xccaa009e, 0x00000000, /* rk5 */
		0x63cd6124, 0x00000001, /* rk6 */
		0xf7011640, 0x00000001, /* rk7 */
		0xdb710640, 0x00000001  /* rk8 */
};
#pragma clang diagnostic pop

constexpr std::array<uint32_t, 4> crc_mask{0xFFFFFFFF, 0xFFFFFFFF, 0x00000000, 0x00000000};

constexpr std::array<uint32_t, 4> crc_mask2{0x00000000, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF};

static uint32_t crc_fold_512to32(crc512& crc_)
{
	const __m128i xmm_mask = _mm_load_si128(reinterpret_cast<const __m128i*>(crc_mask.data()));
	const __m128i xmm_mask2 = _mm_load_si128(reinterpret_cast<const __m128i*>(crc_mask2.data()));

	/*
     * k1
     */
	auto crc_fold = _mm_load_si128(reinterpret_cast<const __m128i*>(crc_k));

	auto x_tmp0 = _mm_clmulepi64_si128(crc_.crcs[0], crc_fold, 0x10);
	crc_.crcs[0] = _mm_clmulepi64_si128(crc_.crcs[0], crc_fold, 0x01);
	crc_.crcs[1] = _mm_xor_si128(crc_.crcs[1], x_tmp0);
	crc_.crcs[1] = _mm_xor_si128(crc_.crcs[1], crc_.crcs[0]);

	auto x_tmp1 = _mm_clmulepi64_si128(crc_.crcs[1], crc_fold, 0x10);
	crc_.crcs[1] = _mm_clmulepi64_si128(crc_.crcs[1], crc_fold, 0x01);
	crc_.crcs[2] = _mm_xor_si128(crc_.crcs[2], x_tmp1);
	crc_.crcs[2] = _mm_xor_si128(crc_.crcs[2], crc_.crcs[1]);

	auto x_tmp2 = _mm_clmulepi64_si128(crc_.crcs[2], crc_fold, 0x10);
	crc_.crcs[2] = _mm_clmulepi64_si128(crc_.crcs[2], crc_fold, 0x01);
	crc_.crcs[3] = _mm_xor_si128(crc_.crcs[3], x_tmp2);
	crc_.crcs[3] = _mm_xor_si128(crc_.crcs[3], crc_.crcs[2]);

	/*
     * k5
     */
	crc_fold = _mm_load_si128(reinterpret_cast<const __m128i*>(crc_k) + 1);

	crc_.crcs[0] = crc_.crcs[3];
	crc_.crcs[3] = _mm_clmulepi64_si128(crc_.crcs[3], crc_fold, 0);
	crc_.crcs[0] = _mm_srli_si128(crc_.crcs[0], 8);
	crc_.crcs[3] = _mm_xor_si128(crc_.crcs[3], crc_.crcs[0]);

	crc_.crcs[0] = crc_.crcs[3];
	crc_.crcs[3] = _mm_slli_si128(crc_.crcs[3], 4);
	crc_.crcs[3] = _mm_clmulepi64_si128(crc_.crcs[3], crc_fold, 0x10);
	crc_.crcs[3] = _mm_xor_si128(crc_.crcs[3], crc_.crcs[0]);
	crc_.crcs[3] = _mm_and_si128(crc_.crcs[3], xmm_mask2);

	/*
     * k7
     */
	crc_.crcs[1] = crc_.crcs[3];
	crc_.crcs[2] = crc_.crcs[3];
	crc_fold = _mm_load_si128(reinterpret_cast<const __m128i*>(crc_k) + 2);

	crc_.crcs[3] = _mm_clmulepi64_si128(crc_.crcs[3], crc_fold, 0);
	crc_.crcs[3] = _mm_xor_si128(crc_.crcs[3], crc_.crcs[2]);
	crc_.crcs[3] = _mm_and_si128(crc_.crcs[3], xmm_mask);

	crc_.crcs[2] = crc_.crcs[3];
	crc_.crcs[3] = _mm_clmulepi64_si128(crc_.crcs[3], crc_fold, 0x10);
	crc_.crcs[3] = _mm_xor_si128(crc_.crcs[3], crc_.crcs[2]);
	crc_.crcs[3] = _mm_xor_si128(crc_.crcs[3], crc_.crcs[1]);

	return ~static_cast<uint32_t>(_mm_extract_epi32(crc_.crcs[3], 2));
}

uint32_t compute(uint32_t /*crc32*/, const unsigned char* data, size_t data_length) noexcept
{
	if (data_length == 0)
		return 0;

	crc512 crc;
	if (data_length < 16) {

		auto partial = _mm_loadu_si128(reinterpret_cast<const __m128i*>(data));
		partial_fold(data_length, crc.crcs[0], crc.crcs[1], crc.crcs[2], crc.crcs[3], partial);
		return crc_fold_512to32(crc);
	}

	const auto byte_blocks = data_length / 64;
	size_t buff_position = 0;

	for (size_t i = 0; i < byte_blocks; i++) {
		auto xmm_t0 = _mm_load_si128(reinterpret_cast<const __m128i*>(&data[buff_position]));
		auto xmm_t1 = _mm_load_si128(reinterpret_cast<const __m128i*>(&data[buff_position]) + 1);
		auto xmm_t2 = _mm_load_si128(reinterpret_cast<const __m128i*>(&data[buff_position]) + 2);
		auto xmm_t3 = _mm_load_si128(reinterpret_cast<const __m128i*>(&data[buff_position]) + 3);

		fold_4(crc);

		crc.crcs[0] = _mm_xor_si128(crc.crcs[0], xmm_t0);
		crc.crcs[1] = _mm_xor_si128(crc.crcs[1], xmm_t1);
		crc.crcs[2] = _mm_xor_si128(crc.crcs[2], xmm_t2);
		crc.crcs[3] = _mm_xor_si128(crc.crcs[3], xmm_t3);

		buff_position += 64;
	}

	if (buff_position == data_length) {
		return crc_fold_512to32(crc);
	}

	const auto byte16_blocks = (data_length - buff_position) / 16;
	std::vector<__m128i> leftovers;
	leftovers.reserve(3);

	for (size_t i = 0; i < byte16_blocks; ++i) {
		leftovers.emplace_back(_mm_load_si128(reinterpret_cast<const __m128i*>(&data[buff_position])));
		buff_position += 16;
	}

	switch (leftovers.size()) {
	case 1:
		fold_1(crc.crcs[0], crc.crcs[1], crc.crcs[2], crc.crcs[3]);

		crc.crcs[3] = _mm_xor_si128(crc.crcs[3], leftovers[0]);
		break;
	case 2:
		fold_2(crc.crcs[0], crc.crcs[1], crc.crcs[2], crc.crcs[3]);

		crc.crcs[2] = _mm_xor_si128(crc.crcs[2], leftovers[0]);
		crc.crcs[3] = _mm_xor_si128(crc.crcs[3], leftovers[1]);
		break;
	case 3:
		fold_3(crc.crcs[0], crc.crcs[1], crc.crcs[2], crc.crcs[3]);

		crc.crcs[1] = _mm_xor_si128(crc.crcs[1], leftovers[0]);
		crc.crcs[2] = _mm_xor_si128(crc.crcs[2], leftovers[1]);
		crc.crcs[3] = _mm_xor_si128(crc.crcs[3], leftovers[2]);
		break;
	}
	if (buff_position < data_length) {
		auto partial = _mm_load_si128(reinterpret_cast<const __m128i*>(&data[buff_position]));
		partial_fold(data_length - buff_position, crc.crcs[0], crc.crcs[1], crc.crcs[2], crc.crcs[3], partial);
		return crc_fold_512to32(crc);
	}
	return crc_fold_512to32(crc);
}
}// namespace CRC32::Intrinsic