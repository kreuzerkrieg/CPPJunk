#include "Crc32.h"
#include "utils/BufferUtils.h"
#include <gtest/gtest.h>
#include <random>
#include <zlib.h>

#include <eve/algo/for_each.hpp>
#include <eve/algo/inclusive_scan.hpp>
#include <eve/function/reduce.hpp>
#include <eve/function/scan.hpp>
#include <eve/views/convert.hpp>
#include <eve/views/zip.hpp>

namespace Crc32Impl = CRC32::Generic;

TEST(Crc32, ComputeOneByte)
{
	std::random_device rd;
	std::mt19937 gen(rd());
	for (int i = 0; i < 1024; ++i) {
		auto buff = createRandomData<std::vector<unsigned char>>(1);
		const auto etalon = crc32(0, buff.data(), buff.size());
		ASSERT_EQ(etalon, Crc32Impl::compute(0, buff.data(), buff.size()));
		ASSERT_EQ(etalon, Crc32Impl::compute_ex(0, buff.data(), buff.size()));
		// ASSERT_EQ(etalon, Crc32Impl::compute_wide<4>(0, buff.data(), buff.size()));
		ASSERT_EQ(etalon, Crc32Impl::compute_wide<8>(0, buff.data(), buff.size()));
		ASSERT_EQ(etalon, Crc32Impl::compute_wide<16>(0, buff.data(), buff.size()));
		ASSERT_EQ(etalon, Crc32Impl::compute_wide<24>(0, buff.data(), buff.size()));
		ASSERT_EQ(etalon, Crc32Impl::compute_wide<32>(0, buff.data(), buff.size()));
	}
}

TEST(Crc32, ComputeShort)
{
	std::random_device rd;
	std::mt19937 gen(rd());
	for (int i = 0; i < 1024; ++i) {
		auto buff = createRandomData<std::vector<unsigned char>>(std::uniform_int_distribution<size_t>(2, 16)(gen));
		const auto etalon = crc32(0, buff.data(), buff.size());
		ASSERT_EQ(etalon, Crc32Impl::compute(0, buff.data(), buff.size()));
		ASSERT_EQ(etalon, Crc32Impl::compute_ex(0, buff.data(), buff.size()));
		// ASSERT_EQ(etalon, Crc32Impl::compute_wide<4>(0, buff.data(), buff.size()));
		ASSERT_EQ(etalon, Crc32Impl::compute_wide<8>(0, buff.data(), buff.size()));
		ASSERT_EQ(etalon, Crc32Impl::compute_wide<16>(0, buff.data(), buff.size()));
		ASSERT_EQ(etalon, Crc32Impl::compute_wide<24>(0, buff.data(), buff.size()));
		ASSERT_EQ(etalon, Crc32Impl::compute_wide<32>(0, buff.data(), buff.size()));
	}
}
TEST(Crc32, ComputeOneKiB)
{
	for (int i = 0; i < 1024; ++i) {
		auto buff = createRandomData<std::vector<unsigned char>>(1024);
		const auto etalon = crc32(0, buff.data(), buff.size());
		ASSERT_EQ(etalon, Crc32Impl::compute(0, buff.data(), buff.size()));
		ASSERT_EQ(etalon, Crc32Impl::compute_ex(0, buff.data(), buff.size()));
		// ASSERT_EQ(etalon, Crc32Impl::compute_wide<4>(0, buff.data(), buff.size()));
		ASSERT_EQ(etalon, Crc32Impl::compute_wide<8>(0, buff.data(), buff.size()));
		ASSERT_EQ(etalon, Crc32Impl::compute_wide<16>(0, buff.data(), buff.size()));
		ASSERT_EQ(etalon, Crc32Impl::compute_wide<24>(0, buff.data(), buff.size()));
		ASSERT_EQ(etalon, Crc32Impl::compute_wide<32>(0, buff.data(), buff.size()));
	}
}

TEST(Crc32, ComputeOneMiB)
{
	for (int i = 0; i < 128; ++i) {
		auto buff = createRandomData<std::vector<unsigned char>>(1024 * 1024);
		const auto etalon = crc32(0, buff.data(), buff.size());
		ASSERT_EQ(etalon, Crc32Impl::compute(0, buff.data(), buff.size()));
		ASSERT_EQ(etalon, Crc32Impl::compute_ex(0, buff.data(), buff.size()));
		// ASSERT_EQ(etalon, Crc32Impl::compute_wide<4>(0, buff.data(), buff.size()));
		ASSERT_EQ(etalon, Crc32Impl::compute_wide<8>(0, buff.data(), buff.size()));
		ASSERT_EQ(etalon, Crc32Impl::compute_wide<16>(0, buff.data(), buff.size()));
		ASSERT_EQ(etalon, Crc32Impl::compute_wide<24>(0, buff.data(), buff.size()));
		ASSERT_EQ(etalon, Crc32Impl::compute_wide<32>(0, buff.data(), buff.size()));
	}
}

TEST(Crc32, ComputeFuzzy)
{
	std::random_device rd;
	std::mt19937 gen(rd());
	for (int i = 0; i < 128; ++i) {
		auto buff = createRandomData<std::vector<unsigned char>>(
				std::uniform_int_distribution<size_t>(1, 1024 * 1024)(gen));
		const auto etalon = crc32(0, buff.data(), buff.size());
		ASSERT_EQ(etalon, Crc32Impl::compute(0, buff.data(), buff.size()));
		ASSERT_EQ(etalon, Crc32Impl::compute_ex(0, buff.data(), buff.size()));
		// ASSERT_EQ(etalon, Crc32Impl::compute_wide<4>(0, buff.data(), buff.size()));
		ASSERT_EQ(etalon, Crc32Impl::compute_wide<8>(0, buff.data(), buff.size()));
		ASSERT_EQ(etalon, Crc32Impl::compute_wide<16>(0, buff.data(), buff.size()));
		ASSERT_EQ(etalon, Crc32Impl::compute_wide<24>(0, buff.data(), buff.size()));
		ASSERT_EQ(etalon, Crc32Impl::compute_wide<32>(0, buff.data(), buff.size()));
	}
}

//TEST(Crc32, CombineQuick)
//{
//	std::random_device rd;
//	std::mt19937 gen(rd());
//
//	uint32_t adler = 0;
//	uint32_t adler_orig = 0;
//	for (int i = 0; i < 16; ++i) {
//		auto buff = createRandomData<std::vector<unsigned char>>(std::uniform_int_distribution<size_t>(1, 1024)(gen));
//		adler_orig = adler32_combine(adler_orig, adler32(0, buff.data(), buff.size()), buff.size());
//		adler = Crc32Impl::combine(adler, Crc32Impl::compute(0, buff.data(), buff.size()), buff.size());
//	}
//	ASSERT_EQ(adler, adler_orig);
//}
//
//TEST(Crc32, CombineFuzzy)
//{
//	std::random_device rd;
//	std::mt19937 gen(rd());
//
//	uint32_t adler = 0;
//	uint32_t adler_orig = 0;
//	for (int i = 0; i < 1024; ++i) {
//		auto buff = createRandomData<std::vector<unsigned char>>(
//				std::uniform_int_distribution<size_t>(1, 16 * 1024 * 1024)(gen));
//		adler_orig = adler32_combine(adler_orig, adler32(0, buff.data(), buff.size()), buff.size());
//		adler = Crc32Impl::combine(adler, Crc32Impl::compute(0, buff.data(), buff.size()), buff.size());
//	}
//	ASSERT_EQ(adler, adler_orig);
//}
