
#include <gmock/gmock.h>


#include <yats/Util.h>


TEST(util_test, is_unique_ptr_test)
{
	ASSERT_TRUE(is_unique_ptr_v<std::unique_ptr<int>>);
	ASSERT_FALSE(is_unique_ptr_v<std::shared_ptr<int>>);
	ASSERT_FALSE(is_unique_ptr_v<int>);
	ASSERT_FALSE(is_unique_ptr_v<int*>);
	ASSERT_FALSE(is_unique_ptr_v<int&>);
	ASSERT_FALSE(is_unique_ptr_v<const int&>);
	ASSERT_FALSE(is_unique_ptr_v<void>);
}

TEST(util_test, is_shared_ptr_test)
{
	ASSERT_TRUE(is_shared_ptr_v<std::shared_ptr<int>>);
	ASSERT_FALSE(is_shared_ptr_v<std::unique_ptr<int>>);
	ASSERT_FALSE(is_shared_ptr_v<int>);
	ASSERT_FALSE(is_shared_ptr_v<int*>);
	ASSERT_FALSE(is_shared_ptr_v<int&>);
	ASSERT_FALSE(is_shared_ptr_v<const int&>);
	ASSERT_FALSE(is_shared_ptr_v<void>);
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

	ASSERT_TRUE(has_run_v<member_run>);
	ASSERT_TRUE(has_run_v<static_run>);
	ASSERT_FALSE(has_run_v<no_run>);
}
