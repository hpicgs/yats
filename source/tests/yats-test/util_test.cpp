#include <gmock/gmock.h>

#include <yats/slot.h>
#include <yats/util.h>

using namespace yats;

TEST(util_test, is_output_bundle)
{
    EXPECT_FALSE(is_output_bundle_v<std::tuple<>>);
    EXPECT_FALSE(is_output_bundle_v<std::tuple<int>>);
    auto val = is_output_bundle_v<output_bundle<int, float>>;
    EXPECT_TRUE(val);

    EXPECT_FALSE(is_output_bundle_v<std::shared_ptr<int>>);
    EXPECT_FALSE(is_output_bundle_v<std::unique_ptr<int>>);
    EXPECT_FALSE(is_output_bundle_v<int>);
    EXPECT_FALSE(is_output_bundle_v<int*>);
    EXPECT_FALSE(is_output_bundle_v<int&>);
    EXPECT_FALSE(is_output_bundle_v<const int&>);
    EXPECT_FALSE(is_output_bundle_v<void>);

    struct no_shared_ptr
    {
        char dummy;
    };

    EXPECT_FALSE(is_output_bundle_v<no_shared_ptr>);
    EXPECT_FALSE(is_output_bundle_v<no_shared_ptr*>);
}

TEST(util_test, has_run_test)
{
    struct static_run
    {
        static float run(float, int, char)
        {
            return 0.f;
        }
    };

    struct member_run
    {
        void run(float, int, char)
        {
        }
    };

    struct no_run
    {
        int i;
    };

    EXPECT_TRUE(has_run_v<member_run>);
    EXPECT_TRUE(has_run_v<static_run>);
    EXPECT_FALSE(has_run_v<no_run>);
}

TEST(util_test, get_value_type_test)
{
    struct Test
    {
    };

    using test_tuple2 = std::tuple<slot<float, 321>, slot<int, 123>>;

    constexpr auto index1 = get_index_by_id_v<321, test_tuple2>;
    constexpr auto index2 = get_index_by_id_v<123, test_tuple2>;

    EXPECT_EQ(index1, 0);
    EXPECT_EQ(index2, 1);
}

TEST(util_test, has_unique_ids_test)
{
    constexpr auto yes = has_unique_ids_v<std::tuple<slot<int, 123>, slot<float, 321>>>;
    constexpr auto no = has_unique_ids_v<std::tuple<slot<int, 123>, slot<float, 123>>>;

    EXPECT_TRUE(yes);
    EXPECT_FALSE(no);

    constexpr auto yes_empty = has_unique_ids_v<std::tuple<>>;

    EXPECT_TRUE(yes_empty);
}
