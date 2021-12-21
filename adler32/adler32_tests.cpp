#include "Adler32.h"
#include "utils/BufferUtils.h"
#include <gtest/gtest.h>
#include <random>
#include <zlib.h>

TEST(Adler, ComputeOneByte)
{
	for (int i = 0; i < 1024; ++i) {
		auto buff = createRandomData<std::vector<unsigned char>>(1);
		ASSERT_EQ(adler32(0, buff.data(), buff.size()), Adler32::compute(0, buff.data(), buff.size()));
	}
}

TEST(Adler, ComputeShort)
{
	std::random_device rd;
	std::mt19937 gen(rd());
	for (int i = 0; i < 1024; ++i) {
		auto buff = createRandomData<std::vector<unsigned char>>(std::uniform_int_distribution<size_t>(2, 16)(gen));
		ASSERT_EQ(adler32(0, buff.data(), buff.size()), Adler32::compute(0, buff.data(), buff.size()));
	}
}
TEST(Adler, ComputeOneKiB)
{
	for (int i = 0; i < 1024; ++i) {
		auto buff = createRandomData<std::vector<unsigned char>>(1024);
		ASSERT_EQ(adler32(0, buff.data(), buff.size()), Adler32::compute(0, buff.data(), buff.size()));
	}
}

TEST(Adler, ComputeOneMiB)
{
	for (int i = 0; i < 1024; ++i) {
		auto buff = createRandomData<std::vector<unsigned char>>(1024 * 1024);
		ASSERT_EQ(adler32(0, buff.data(), buff.size()), Adler32::compute(0, buff.data(), buff.size()));
	}
}

TEST(Adler, ComputeFuzzy)
{
	std::random_device rd;
	std::mt19937 gen(rd());
	for (int i = 0; i < 1024; ++i) {
		auto buff = createRandomData<std::vector<unsigned char>>(
				std::uniform_int_distribution<size_t>(1, 16 * 1024 * 1024)(gen));
		ASSERT_EQ(adler32(0, buff.data(), buff.size()), Adler32::compute(0, buff.data(), buff.size()));
	}
}

TEST(Adler, CombineQuick)
{
	std::random_device rd;
	std::mt19937 gen(rd());

	uint32_t adler = 0;
	uint32_t adler_orig = 0;
	for (int i = 0; i < 16; ++i) {
		auto buff = createRandomData<std::vector<unsigned char>>(std::uniform_int_distribution<size_t>(1, 1024)(gen));
		adler_orig = adler32_combine(adler_orig, adler32(0, buff.data(), buff.size()), buff.size());
		adler = Adler32::combine(adler, Adler32::compute(0, buff.data(), buff.size()), buff.size());
	}
	ASSERT_EQ(adler, adler_orig);
}

TEST(Adler, CombineFuzzy)
{
	std::random_device rd;
	std::mt19937 gen(rd());

	uint32_t adler = 0;
	uint32_t adler_orig = 0;
	for (int i = 0; i < 1024; ++i) {
		auto buff = createRandomData<std::vector<unsigned char>>(
				std::uniform_int_distribution<size_t>(1, 16 * 1024 * 1024)(gen));
		adler_orig = adler32_combine(adler_orig, adler32(0, buff.data(), buff.size()), buff.size());
		adler = Adler32::combine(adler, Adler32::compute(0, buff.data(), buff.size()), buff.size());
	}
	ASSERT_EQ(adler, adler_orig);
}