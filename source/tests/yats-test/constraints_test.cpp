#include <gmock/gmock.h>

#include <yats/constraint.h>

using namespace yats;

TEST(constraints_test, remove_any_thread)
{
    auto constraint = thread_group::any_thread();

    {
        bool contains_any = constraint.names().count(thread_group::any_thread_name()) == 1;
        EXPECT_TRUE(contains_any);
    }

    constraint |= thread_group::main_thread();

    {
        bool contains_any = constraint.names().count(thread_group::any_thread_name()) == 1;
        EXPECT_FALSE(contains_any);
    }

}

TEST(constraints_test, no_duplicate_threads)
{
    auto constraint = thread_group("one") | thread_group("two") | thread_group("one");

    bool contains_one = constraint.names().count("one") == 1;
    bool contains_two = constraint.names().count("two") == 1;
    EXPECT_TRUE(contains_one);
    EXPECT_TRUE(contains_two);

}
