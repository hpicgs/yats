#include <gmock/gmock.h>

#include <memory>

#include <yats/queue.h>

TEST(queue_test, construct_nothrow)
{
    EXPECT_NO_THROW(yats::queue<int>());
    EXPECT_NO_THROW(yats::queue<std::unique_ptr<int>>());
}

TEST(queue_test, correct_size)
{
    yats::queue<int> queue;
    EXPECT_EQ(queue.size(), 0);

    queue.push(1);
    EXPECT_EQ(queue.size(), 1);

    auto value = queue.extract();
    EXPECT_EQ(queue.size(), 0);
    EXPECT_EQ(value, 1);
}

TEST(queue_test, push_extract_no_copy)
{
    yats::queue<std::unique_ptr<int>> queue;
    EXPECT_EQ(queue.size(), 0);

    queue.push(std::make_unique<int>(42));
    EXPECT_EQ(queue.size(), 1);

    auto unique_ptr = queue.extract();
    EXPECT_EQ(queue.size(), 0);
    EXPECT_EQ(*unique_ptr, 42);
}
