#include <memory>

#include <gmock/gmock.h>

#include <yats/thread_safe_queue.h>

using namespace yats;

TEST(queue_test, correct_size)
{
    thread_safe_queue<int> queue;
    EXPECT_TRUE(queue.empty());

    queue.push(1);
    EXPECT_FALSE(queue.empty());

    auto value = queue.extract();
    EXPECT_TRUE(queue.empty());
    EXPECT_EQ(value, 1);
}

TEST(queue_test, push_extract_no_copy)
{
    thread_safe_queue<std::unique_ptr<int>> queue;
    EXPECT_TRUE(queue.empty());

    queue.push(std::make_unique<int>(42));
    EXPECT_FALSE(queue.empty());

    auto unique_ptr = queue.extract();
    EXPECT_TRUE(queue.empty());
    EXPECT_EQ(*unique_ptr, 42);
}
