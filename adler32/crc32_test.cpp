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

//TEST(Crc32, ComputeOneByte)
//{
//	auto foo = CRCTables;
//	std::cout << eve::expected_cardinal_v<unsigned char> << std::endl;
////	std::cout << typeid(eve::cardinal<unsigned char>::type).name() << std::endl;
//
//	auto buff = createRandomData<std::vector<unsigned char>>(4);
//	CRC32::Generic::compute(0, buff.data(), buff.size());
//	std::cout << "=====================================================" << std::endl;
//	std::vector<unsigned char> buff1{0};
//	std::copy(buff.begin(), buff.end(), std::back_inserter(buff1));
//	auto in_as_uint32 = eve::views::convert(buff, eve::as<std::uint32_t>{});
//	constexpr std::uint32_t zero{0};
//	eve::wide<uint32_t> retval{0};
//	//	eve::views::zip_range zipped = eve::views::zip(buff, buff1);
//	//	eve::algo::inclusive_scan_to(buff, buff1, std::pair(std::plus(), eve::zero));
//
//	eve::algo::for_each(in_as_uint32, [&](auto iter, auto ignore) {
//		eve::wide<std::uint32_t> chunk = eve::load[ignore.else_(zero)](iter);
//		std::cout << "chunk = " << chunk << std::endl;
//		retval = eve::reduce(chunk, [](auto init, auto val) {
//			//			const eve::wide<uint32_t> ff{0xff};
//			eve::wide<std::uint32_t> lookup_index{0};
//			lookup_index = eve::scan(
//					val,
//					[](auto val1, auto init1) {
//						std::cout << "init1 = " << init1 << std::endl;
//						std::cout << "val1 = " << val1 << std::endl;
//						//				const eve::wide<uint32_t> ff{};
//						return init1 ^ val1 & 0xff;
//					},
//					lookup_index);
//			std::cout << "lookup_index = " << lookup_index << std::endl;
//			std::cout << "init = " << init << std::endl;
//			std::cout << "val = " << val << std::endl;
//			return (val >> 8) ^ eve::wide<std::uint32_t>{CRCTable[lookup_index.get(0)], CRCTable[lookup_index.get(1)],
//														 CRCTable[lookup_index.get(2)], CRCTable[lookup_index.get(3)]};
//			//return a + b;
//		});
//		std::cout << "retval = " << (retval ^ 0xFFFFFFFF) << std::endl;
//
//		//		retval = eve::scan(chunk, eve::plus, retval);
//		//		retval = eve::scan(chunk, eve::plus, retval);
//	});
//	retval ^= 0xFFFFFFFF;
//	std::cout << "retval = " << retval << std::endl;
//	std::cout << "=====================================================" << std::endl;
//	//	ASSERT_EQ(buff1.back(), 2 * buff[0] + 2 * buff[1] + 2 * buff[2]);
//	ASSERT_EQ(retval.back(), CRC32::Generic::compute(0, buff.data(), buff.size()));
//	/*for (int i = 0; i < 1024; ++i) {
//		auto buff = createRandomData<std::vector<unsigned char>>(1);
//		ASSERT_EQ(crc32(0, buff.data(), buff.size()), Crc32Impl::compute(0, buff.data(), buff.size()));
//	}*/
//}

TEST(Crc32, ComputeShort)
{
	std::random_device rd;
	std::mt19937 gen(rd());
	for (int i = 0; i < 1024; ++i) {
		auto buff = createRandomData<std::vector<unsigned char>>(std::uniform_int_distribution<size_t>(2, 16)(gen));
		ASSERT_EQ(crc32(0, buff.data(), buff.size()), Crc32Impl::compute(0, buff.data(), buff.size()));
		ASSERT_EQ(Crc32Impl::compute(0, buff.data(), buff.size()), Crc32Impl::compute_ex(0, buff.data(), buff.size()));
	}
}
TEST(Crc32, ComputeOneKiB)
{
	for (int i = 0; i < 1024; ++i) {
		auto buff = createRandomData<std::vector<unsigned char>>(1024);
		ASSERT_EQ(crc32(0, buff.data(), buff.size()), Crc32Impl::compute(0, buff.data(), buff.size()));
		ASSERT_EQ(Crc32Impl::compute(0, buff.data(), buff.size()), Crc32Impl::compute_ex(0, buff.data(), buff.size()));
	}
}

TEST(Crc32, ComputeOneMiB)
{
	for (int i = 0; i < 128; ++i) {
		auto buff = createRandomData<std::vector<unsigned char>>(1024 * 1024);
		ASSERT_EQ(crc32(0, buff.data(), buff.size()), Crc32Impl::compute(0, buff.data(), buff.size()));
		ASSERT_EQ(Crc32Impl::compute(0, buff.data(), buff.size()), Crc32Impl::compute_ex(0, buff.data(), buff.size()));
	}
}

TEST(Crc32, ComputeFuzzy)
{
	std::random_device rd;
	std::mt19937 gen(rd());
	for (int i = 0; i < 128; ++i) {
		auto buff = createRandomData<std::vector<unsigned char>>(
				std::uniform_int_distribution<size_t>(1, 16 * 1024 * 1024)(gen));
		ASSERT_EQ(crc32(0, buff.data(), buff.size()), Crc32Impl::compute(0, buff.data(), buff.size()));
		ASSERT_EQ(Crc32Impl::compute(0, buff.data(), buff.size()), Crc32Impl::compute_ex(0, buff.data(), buff.size()));
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
