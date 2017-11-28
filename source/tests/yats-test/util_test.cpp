
#include <gmock/gmock.h>


#include <yats/Util.h>


TEST(is_unique_ptr_test, util_test)
{
	ASSERT_TRUE(is_unique_ptr_v<std::unique_ptr<int>>);
	ASSERT_FALSE(is_unique_ptr_v<std::shared_ptr<int>>);
	ASSERT_FALSE(is_unique_ptr_v<int>);
}

TEST(is_shared_ptr_test, util_test)
{
	ASSERT_TRUE(is_shared_ptr_v<std::shared_ptr<int>>);
	ASSERT_FALSE(is_shared_ptr_v<std::unique_ptr<int>>);
	ASSERT_FALSE(is_shared_ptr_v<int>);
}

TEST(has_options_test, util_test)
{
	struct static_options
	{
		//TODO: temporary as long as we don't have any Option object
		static has_options<int>::Option options() { return has_options<int>::Option(); }
	};

	struct member_options
	{
		//TODO: temporary as long as we don't have any Option object
		has_options<int>::Option options() { return has_options<int>::Option(); }
	};

	struct no_options
	{
		int i;
	};

	ASSERT_TRUE(has_options_v<static_options>);
	ASSERT_FALSE(has_options_v<member_options>);
	ASSERT_FALSE(has_options_v<no_options>);

}

TEST(has_run_test, util_test)
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
