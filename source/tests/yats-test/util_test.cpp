#include <gmock/gmock.h>

#include <yats/Util.h>
#include <yats/Input.h>
#include <yats/Output.h>


TEST(util_test, is_unique_ptr_test)
{
	EXPECT_TRUE(yats::is_unique_ptr_v<std::unique_ptr<int>>);
	EXPECT_FALSE(yats::is_unique_ptr_v<std::shared_ptr<int>>);
	EXPECT_FALSE(yats::is_unique_ptr_v<int>);
	EXPECT_FALSE(yats::is_unique_ptr_v<int*>);
	EXPECT_FALSE(yats::is_unique_ptr_v<int&>);
	EXPECT_FALSE(yats::is_unique_ptr_v<const int&>);
	EXPECT_FALSE(yats::is_unique_ptr_v<void>);

	struct no_unique_ptr
	{
		char dummy;
	};

	EXPECT_FALSE(yats::is_unique_ptr_v<no_unique_ptr>);
	EXPECT_FALSE(yats::is_unique_ptr_v<no_unique_ptr*>);
}

TEST(util_test, is_shared_ptr_test)
{
	EXPECT_TRUE(yats::is_shared_ptr_v<std::shared_ptr<int>>);
	EXPECT_FALSE(yats::is_shared_ptr_v<std::unique_ptr<int>>);
	EXPECT_FALSE(yats::is_shared_ptr_v<int>);
	EXPECT_FALSE(yats::is_shared_ptr_v<int*>);
	EXPECT_FALSE(yats::is_shared_ptr_v<int&>);
	EXPECT_FALSE(yats::is_shared_ptr_v<const int&>);
	EXPECT_FALSE(yats::is_shared_ptr_v<void>);

	struct no_shared_ptr
	{
		char dummy;
	};

	EXPECT_FALSE(yats::is_unique_ptr_v<no_shared_ptr>);
	EXPECT_FALSE(yats::is_unique_ptr_v<no_shared_ptr*>);
}

TEST(util_test, has_run_test)
{
	struct static_run
	{
		static float run(float, int, char) { return 0.f; }
	};

	struct member_run
	{
		void run(float, int, char) {}
	};

	struct no_run
	{
		int i;
	};

	EXPECT_TRUE(yats::has_run_v<member_run>);
	EXPECT_TRUE(yats::has_run_v<static_run>);
	EXPECT_FALSE(yats::has_run_v<no_run>);
}

TEST(util_test, get_value_type_test)
{
	struct Test {};

	using TestTuple1 = yats::OutputBundle<yats::Output<Test, 0>>;
	using TestTuple2 = std::tuple<yats::Input<float, 321>, yats::Input<int, 123>>;

	constexpr auto index1 = yats::get_index_by_id_v<321, TestTuple2>;
	constexpr auto index2 = yats::get_index_by_id_v<123, TestTuple2>;

	EXPECT_EQ(index1, 0);
	EXPECT_EQ(index2, 1);
}

TEST(util_test, has_unique_ids_test)
{
	constexpr auto yes = yats::has_unique_ids_v<std::tuple<yats::Input<int, 123>, yats::Input<float, 321>>>;
	constexpr auto no = yats::has_unique_ids_v<std::tuple<yats::Output<int, 123>, yats::Output<float, 123>>>;

	EXPECT_TRUE(yes);
	EXPECT_FALSE(no);
}
