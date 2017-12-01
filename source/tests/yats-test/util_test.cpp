
#include <gmock/gmock.h>


#include <yats/Util.h>


TEST(util_test, is_unique_ptr_test)
{
	EXPECT_TRUE(is_unique_ptr_v<std::unique_ptr<int>>);
	EXPECT_FALSE(is_unique_ptr_v<std::shared_ptr<int>>);
	EXPECT_FALSE(is_unique_ptr_v<int>);
	EXPECT_FALSE(is_unique_ptr_v<int*>);
	EXPECT_FALSE(is_unique_ptr_v<int&>);
	EXPECT_FALSE(is_unique_ptr_v<const int&>);
	EXPECT_FALSE(is_unique_ptr_v<void>);

	struct no_unique_ptr
	{
		char dummy;
	};

	EXPECT_FALSE(is_unique_ptr_v<no_unique_ptr>);
	EXPECT_FALSE(is_unique_ptr_v<no_unique_ptr*>);
}

TEST(util_test, is_shared_ptr_test)
{
	EXPECT_TRUE(is_shared_ptr_v<std::shared_ptr<int>>);
	EXPECT_FALSE(is_shared_ptr_v<std::unique_ptr<int>>);
	EXPECT_FALSE(is_shared_ptr_v<int>);
	EXPECT_FALSE(is_shared_ptr_v<int*>);
	EXPECT_FALSE(is_shared_ptr_v<int&>);
	EXPECT_FALSE(is_shared_ptr_v<const int&>);
	EXPECT_FALSE(is_shared_ptr_v<void>);

	struct no_shared_ptr
	{
		char dummy;
	};

	EXPECT_FALSE(is_unique_ptr_v<no_shared_ptr>);
	EXPECT_FALSE(is_unique_ptr_v<no_shared_ptr*>);
}

TEST(util_test, has_run_test)
{
	struct static_run
	{
		static float run(float, int, char) {}
	};

	struct member_run
	{
		void run(float, int, char) {}
	};

	struct no_run
	{
		int i;
	};

	EXPECT_TRUE(has_run_v<member_run>);
	EXPECT_TRUE(has_run_v<static_run>);
	EXPECT_FALSE(has_run_v<no_run>);
}
