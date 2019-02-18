#include "gtest/gtest.h"

struct foo
{
	int bar()
	{
		try {
			throw std::runtime_error("Jopa");
		}
		catch (...) {
			std::rethrow_exception(std::current_exception());
		}
	}
};

TEST(foo_test, out_of_range)
{
	foo f;
	EXPECT_THROW(f.bar(), std::runtime_error);
}

int main(int argc, char** argv)
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}