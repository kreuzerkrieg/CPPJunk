#include "Crc32.h"
#include <eve/algo/for_each.hpp>
#include <eve/algo/transform.hpp>
#include <eve/eve.hpp>
#include <eve/function/scan.hpp>
#include <eve/views/convert.hpp>
#include <eve/views/zip.hpp>
#include <span>

/*namespace CRC32::Intrinsic {
const __m128i xmm_fold4 = _mm_set_epi32(0x00000001, 0x54442bd4, 0x00000001, static_cast<int>(0xc6e41596));

static void fold_1(__m128i& xmm_crc0, __m128i& xmm_crc1, __m128i& xmm_crc2, __m128i& xmm_crc3)
{
	__m128i x_tmp3;
	__m128 ps_crc0;
	__m128 ps_crc3;
	__m128 ps_res;

	x_tmp3 = xmm_crc3;

	xmm_crc3 = xmm_crc0;
	xmm_crc0 = _mm_clmulepi64_si128(xmm_crc0, xmm_fold4, 0x01);
	xmm_crc3 = _mm_clmulepi64_si128(xmm_crc3, xmm_fold4, 0x10);
	ps_crc0 = _mm_castsi128_ps(xmm_crc0);
	ps_crc3 = _mm_castsi128_ps(xmm_crc3);
	ps_res = _mm_xor_ps(ps_crc0, ps_crc3);

	xmm_crc0 = xmm_crc1;
	xmm_crc1 = xmm_crc2;
	xmm_crc2 = x_tmp3;
	xmm_crc3 = _mm_castps_si128(ps_res);
}

*//*static void fold_2(__m128i& xmm_crc0, __m128i& xmm_crc1, __m128i& xmm_crc2, __m128i& xmm_crc3)
{
	__m128i x_tmp3;
	__m128i x_tmp2;

	__m128 ps_crc0;
	__m128 ps_crc1;
	__m128 ps_crc2;
	__m128 ps_crc3;
	__m128 ps_res31;
	__m128 ps_res20;

	x_tmp3 = xmm_crc3;
	x_tmp2 = xmm_crc2;

	xmm_crc3 = xmm_crc1;
	xmm_crc1 = _mm_clmulepi64_si128(xmm_crc1, xmm_fold4, 0x01);
	xmm_crc3 = _mm_clmulepi64_si128(xmm_crc3, xmm_fold4, 0x10);
	ps_crc3 = _mm_castsi128_ps(xmm_crc3);
	ps_crc1 = _mm_castsi128_ps(xmm_crc1);
	ps_res31 = _mm_xor_ps(ps_crc3, ps_crc1);

	xmm_crc2 = xmm_crc0;
	xmm_crc0 = _mm_clmulepi64_si128(xmm_crc0, xmm_fold4, 0x01);
	xmm_crc2 = _mm_clmulepi64_si128(xmm_crc2, xmm_fold4, 0x10);
	ps_crc0 = _mm_castsi128_ps(xmm_crc0);
	ps_crc2 = _mm_castsi128_ps(xmm_crc2);
	ps_res20 = _mm_xor_ps(ps_crc0, ps_crc2);

	xmm_crc0 = x_tmp2;
	xmm_crc1 = x_tmp3;
	xmm_crc2 = _mm_castps_si128(ps_res20);
	xmm_crc3 = _mm_castps_si128(ps_res31);
}

static void fold_3(__m128i& xmm_crc0, __m128i& xmm_crc1, __m128i& xmm_crc2, __m128i& xmm_crc3)
{
	__m128i x_tmp3;
	__m128 ps_crc0;
	__m128 ps_crc1;
	__m128 ps_crc2;
	__m128 ps_crc3;
	__m128 ps_res32;
	__m128 ps_res21;
	__m128 ps_res10;

	x_tmp3 = xmm_crc3;

	xmm_crc3 = xmm_crc2;
	xmm_crc2 = _mm_clmulepi64_si128(xmm_crc2, xmm_fold4, 0x01);
	xmm_crc3 = _mm_clmulepi64_si128(xmm_crc3, xmm_fold4, 0x10);
	ps_crc2 = _mm_castsi128_ps(xmm_crc2);
	ps_crc3 = _mm_castsi128_ps(xmm_crc3);
	ps_res32 = _mm_xor_ps(ps_crc2, ps_crc3);

	xmm_crc2 = xmm_crc1;
	xmm_crc1 = _mm_clmulepi64_si128(xmm_crc1, xmm_fold4, 0x01);
	xmm_crc2 = _mm_clmulepi64_si128(xmm_crc2, xmm_fold4, 0x10);
	ps_crc1 = _mm_castsi128_ps(xmm_crc1);
	ps_crc2 = _mm_castsi128_ps(xmm_crc2);
	ps_res21 = _mm_xor_ps(ps_crc1, ps_crc2);

	xmm_crc1 = xmm_crc0;
	xmm_crc0 = _mm_clmulepi64_si128(xmm_crc0, xmm_fold4, 0x01);
	xmm_crc1 = _mm_clmulepi64_si128(xmm_crc1, xmm_fold4, 0x10);
	ps_crc0 = _mm_castsi128_ps(xmm_crc0);
	ps_crc1 = _mm_castsi128_ps(xmm_crc1);
	ps_res10 = _mm_xor_ps(ps_crc0, ps_crc1);

	xmm_crc0 = x_tmp3;
	xmm_crc1 = _mm_castps_si128(ps_res10);
	xmm_crc2 = _mm_castps_si128(ps_res21);
	xmm_crc3 = _mm_castps_si128(ps_res32);
}

static void fold_4(__m128i& xmm_crc0, __m128i& xmm_crc1, __m128i& xmm_crc2, __m128i& xmm_crc3)
{
	auto x_tmp0 = xmm_crc0;
	auto x_tmp1 = xmm_crc1;
	auto x_tmp2 = xmm_crc2;
	auto x_tmp3 = xmm_crc3;

	xmm_crc0 = _mm_clmulepi64_si128(xmm_crc0, xmm_fold4, 0x01);
	x_tmp0 = _mm_clmulepi64_si128(x_tmp0, xmm_fold4, 0x10);
	auto ps_res0 = _mm_xor_ps(_mm_castsi128_ps(xmm_crc0), _mm_castsi128_ps(x_tmp0));

	xmm_crc1 = _mm_clmulepi64_si128(xmm_crc1, xmm_fold4, 0x01);
	x_tmp1 = _mm_clmulepi64_si128(x_tmp1, xmm_fold4, 0x10);
	auto ps_res1 = _mm_xor_ps(_mm_castsi128_ps(xmm_crc1), _mm_castsi128_ps(x_tmp1));

	xmm_crc2 = _mm_clmulepi64_si128(xmm_crc2, xmm_fold4, 0x01);
	x_tmp2 = _mm_clmulepi64_si128(x_tmp2, xmm_fold4, 0x10);
	auto ps_res2 = _mm_xor_ps(_mm_castsi128_ps(xmm_crc2), _mm_castsi128_ps(x_tmp2));

	xmm_crc3 = _mm_clmulepi64_si128(xmm_crc3, xmm_fold4, 0x01);
	x_tmp3 = _mm_clmulepi64_si128(x_tmp3, xmm_fold4, 0x10);
	auto ps_res3 = _mm_xor_ps(_mm_castsi128_ps(xmm_crc3), _mm_castsi128_ps(x_tmp3));
	xmm_crc0 = _mm_castps_si128(ps_res0);
	xmm_crc1 = _mm_castps_si128(ps_res1);
	xmm_crc2 = _mm_castps_si128(ps_res2);
	xmm_crc3 = _mm_castps_si128(ps_res3);
}*//*

constexpr unsigned __attribute__((aligned(32))) pshufb_shf_table[60] = {
		0x84838281, 0x88878685, 0x8c8b8a89, 0x008f8e8d, *//* shl 15 (16 - 1)/shr1 *//*
		0x85848382, 0x89888786, 0x8d8c8b8a, 0x01008f8e, *//* shl 14 (16 - 3)/shr2 *//*
		0x86858483, 0x8a898887, 0x8e8d8c8b, 0x0201008f, *//* shl 13 (16 - 4)/shr3 *//*
		0x87868584, 0x8b8a8988, 0x8f8e8d8c, 0x03020100, *//* shl 12 (16 - 4)/shr4 *//*
		0x88878685, 0x8c8b8a89, 0x008f8e8d, 0x04030201, *//* shl 11 (16 - 5)/shr5 *//*
		0x89888786, 0x8d8c8b8a, 0x01008f8e, 0x05040302, *//* shl 10 (16 - 6)/shr6 *//*
		0x8a898887, 0x8e8d8c8b, 0x0201008f, 0x06050403, *//* shl  9 (16 - 7)/shr7 *//*
		0x8b8a8988, 0x8f8e8d8c, 0x03020100, 0x07060504, *//* shl  8 (16 - 8)/shr8 *//*
		0x8c8b8a89, 0x008f8e8d, 0x04030201, 0x08070605, *//* shl  7 (16 - 9)/shr9 *//*
		0x8d8c8b8a, 0x01008f8e, 0x05040302, 0x09080706, *//* shl  6 (16 -10)/shr10*//*
		0x8e8d8c8b, 0x0201008f, 0x06050403, 0x0a090807, *//* shl  5 (16 -11)/shr11*//*
		0x8f8e8d8c, 0x03020100, 0x07060504, 0x0b0a0908, *//* shl  4 (16 -12)/shr12*//*
		0x008f8e8d, 0x04030201, 0x08070605, 0x0c0b0a09, *//* shl  3 (16 -13)/shr13*//*
		0x01008f8e, 0x05040302, 0x09080706, 0x0d0c0b0a, *//* shl  2 (16 -14)/shr14*//*
		0x0201008f, 0x06050403, 0x0a090807, 0x0e0d0c0b  *//* shl  1 (16 -15)/shr15*//*
};

static void partial_fold(size_t len, __m128i& xmm_crc0, __m128i& xmm_crc1, __m128i& xmm_crc2, __m128i& xmm_crc3,
						 __m128i& xmm_crc_part)
{
	const __m128i xmm_mask3 = _mm_set1_epi32(static_cast<int>(0x80808080));
	__m128i xmm_shl;
	__m128i xmm_shr;
	__m128i xmm_tmp1;
	__m128i xmm_tmp2;
	__m128i xmm_tmp3;
	__m128i xmm_a0_0;
	__m128i xmm_a0_1;

	__m128 ps_crc3;
	__m128i psa0_0;
	__m128i psa0_1;
	__m128i ps_res;

	xmm_shl = _mm_load_si128(reinterpret_cast<const __m128i*>(pshufb_shf_table) + (len - 1));
	xmm_shr = xmm_shl;
	xmm_shr = _mm_xor_si128(xmm_shr, xmm_mask3);

	xmm_a0_0 = _mm_shuffle_epi8(xmm_crc0, xmm_shl);

	xmm_crc0 = _mm_shuffle_epi8(xmm_crc0, xmm_shr);
	xmm_tmp1 = _mm_shuffle_epi8(xmm_crc1, xmm_shl);
	xmm_crc0 = _mm_or_si128(xmm_crc0, xmm_tmp1);

	xmm_crc1 = _mm_shuffle_epi8(xmm_crc1, xmm_shr);
	xmm_tmp2 = _mm_shuffle_epi8(xmm_crc2, xmm_shl);
	xmm_crc1 = _mm_or_si128(xmm_crc1, xmm_tmp2);

	xmm_crc2 = _mm_shuffle_epi8(xmm_crc2, xmm_shr);
	xmm_tmp3 = _mm_shuffle_epi8(xmm_crc3, xmm_shl);
	xmm_crc2 = _mm_or_si128(xmm_crc2, xmm_tmp3);

	xmm_crc3 = _mm_shuffle_epi8(xmm_crc3, xmm_shr);
	xmm_crc_part = _mm_shuffle_epi8(xmm_crc_part, xmm_shl);
	xmm_crc3 = _mm_or_si128(xmm_crc3, xmm_crc_part);

	xmm_a0_1 = _mm_clmulepi64_si128(xmm_a0_0, xmm_fold4, 0x10);
	xmm_a0_0 = _mm_clmulepi64_si128(xmm_a0_0, xmm_fold4, 0x01);

	ps_crc3 = _mm_castsi128_ps(xmm_crc3);
	psa0_0 = _mm_castsi128_ps(xmm_a0_0);
	psa0_1 = _mm_castsi128_ps(xmm_a0_1);

	ps_res = _mm_xor_ps(ps_crc3, psa0_0);
	ps_res = _mm_xor_ps(ps_res, psa0_1);

	xmm_crc3 = _mm_castps_si128(ps_res);
}

constexpr std::array<uint32_t, 12> crc_k[] = {
		0xccaa009e, 0x00000000, *//* rk1 *//*
		0x751997d0, 0x00000001, *//* rk2 *//*
		0xccaa009e, 0x00000000, *//* rk5 *//*
		0x63cd6124, 0x00000001, *//* rk6 *//*
		0xf7011640, 0x00000001, *//* rk7 *//*
		0xdb710640, 0x00000001  *//* rk8 *//*
};

constexpr std::array<uint32_t, 4> crc_mask{0xFFFFFFFF, 0xFFFFFFFF, 0x00000000, 0x00000000};

constexpr std::array<uint32_t, 4> crc_mask2{0x00000000, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF};

struct crc {
	__m128i crc0 = _mm_cvtsi32_si128(static_cast<int>(0x9db42487));
	__m128i crc1 = _mm_setzero_si128();
	__m128i crc2 = _mm_setzero_si128();
	__m128i crc3 = _mm_setzero_si128();
	__m128i crc_part;
};
static uint32_t crc_fold_512to32(crc& crc_)
{
	const __m128i xmm_mask = _mm_load_si128(reinterpret_cast<const __m128i*>(crc_mask.data()));
	const __m128i xmm_mask2 = _mm_load_si128(reinterpret_cast<const __m128i*>(crc_mask2.data()));

	__m128i x_tmp0;
	__m128i x_tmp1;
	__m128i x_tmp2;
	__m128i crc_fold;

	*//*
     k1
    *//*
	crc_fold = _mm_load_si128(reinterpret_cast<const __m128i*>(crc_k));

	x_tmp0 = _mm_clmulepi64_si128(crc_.crc0, crc_fold, 0x10);
	crc_.crc0 = _mm_clmulepi64_si128(crc_.crc0, crc_fold, 0x01);
	crc_.crc1 = _mm_xor_si128(crc_.crc1, x_tmp0);
	crc_.crc1 = _mm_xor_si128(crc_.crc1, crc_.crc0);

	x_tmp1 = _mm_clmulepi64_si128(crc_.crc1, crc_fold, 0x10);
	crc_.crc1 = _mm_clmulepi64_si128(crc_.crc1, crc_fold, 0x01);
	crc_.crc2 = _mm_xor_si128(crc_.crc2, x_tmp1);
	crc_.crc2 = _mm_xor_si128(crc_.crc2, crc_.crc1);

	x_tmp2 = _mm_clmulepi64_si128(crc_.crc2, crc_fold, 0x10);
	crc_.crc2 = _mm_clmulepi64_si128(crc_.crc2, crc_fold, 0x01);
	crc_.crc3 = _mm_xor_si128(crc_.crc3, x_tmp2);
	crc_.crc3 = _mm_xor_si128(crc_.crc3, crc_.crc2);

	*//*
     * k5
     *//*
	crc_fold = _mm_load_si128(reinterpret_cast<const __m128i*>(crc_k) + 1);

	crc_.crc0 = crc_.crc3;
	crc_.crc3 = _mm_clmulepi64_si128(crc_.crc3, crc_fold, 0);
	crc_.crc0 = _mm_srli_si128(crc_.crc0, 8);
	crc_.crc3 = _mm_xor_si128(crc_.crc3, crc_.crc0);

	crc_.crc0 = crc_.crc3;
	crc_.crc3 = _mm_slli_si128(crc_.crc3, 4);
	crc_.crc3 = _mm_clmulepi64_si128(crc_.crc3, crc_fold, 0x10);
	crc_.crc3 = _mm_xor_si128(crc_.crc3, crc_.crc0);
	crc_.crc3 = _mm_and_si128(crc_.crc3, xmm_mask2);

	*//*
     * k7
     *//*
	crc_.crc1 = crc_.crc3;
	crc_.crc2 = crc_.crc3;
	crc_fold = _mm_load_si128(reinterpret_cast<const __m128i*>(crc_k) + 2);

	crc_.crc3 = _mm_clmulepi64_si128(crc_.crc3, crc_fold, 0);
	crc_.crc3 = _mm_xor_si128(crc_.crc3, crc_.crc2);
	crc_.crc3 = _mm_and_si128(crc_.crc3, xmm_mask);

	crc_.crc2 = crc_.crc3;
	crc_.crc3 = _mm_clmulepi64_si128(crc_.crc3, crc_fold, 0x10);
	crc_.crc3 = _mm_xor_si128(crc_.crc3, crc_.crc2);
	crc_.crc3 = _mm_xor_si128(crc_.crc3, crc_.crc1);

	return ~static_cast<uint32_t>(_mm_extract_epi32(crc_.crc3, 2));
}

}// namespace CRC32::Intrinsic*/

namespace CRC32::SIMD {

uint32_t compute(uint32_t crc32, const unsigned char* /*data*/, size_t /*data_length*/) noexcept
{
	/*Intrinsic::crc crc0;

	const std::uint32_t zero{0};
	std::span buffer_view(data, data_length);
	auto in_as_uint32 = eve::views::convert(buffer_view, eve::as<std::uint64_t>{});

	eve::algo::for_each(in_as_uint32, [&](auto iter, auto ignore) {
		// load the value. if some elements should be ignored, replace them with zero
		eve::wide<std::uint64_t> xs = eve::load[ignore.else_(zero)](iter);
		Intrinsic::fold_1(crc0.crc0, crc0.crc1, crc0.crc2, crc0.crc3);
		crc0.crc3 = _mm_xor_si128(crc0.crc3, xs);
		*//*xmm_t0 = _mm_load_si128(reinterpret_cast<const __m128i*>(&data[buff_position]));
		xmm_t1 = _mm_load_si128(reinterpret_cast<const __m128i*>(&data[buff_position]) + 1);
		xmm_t2 = _mm_load_si128(reinterpret_cast<const __m128i*>(&data[buff_position]) + 2);
		xmm_t3 = _mm_load_si128(reinterpret_cast<const __m128i*>(&data[buff_position]) + 3);

		fold_4(crc0.crc0, crc0.crc1, crc0.crc2, crc0.crc3);

		crc0.crc0 = _mm_xor_si128(crc0.crc0, xmm_t0);
		crc0.crc1 = _mm_xor_si128(crc0.crc1, xmm_t1);
		crc0.crc2 = _mm_xor_si128(crc0.crc2, xmm_t2);
		crc0.crc3 = _mm_xor_si128(crc0.crc3, xmm_t3);*//*
	});*/

	return crc32;//crc_fold_512to32(crc0);
}
}// namespace CRC32::SIMD
