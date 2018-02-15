#include <memory>

#include <gmock/gmock.h>

#include <yats/thread_safe_queue.h>

using namespace yats;

TEST(queue_test, correct_size)
{
    thread_safe_queue<int> queue;
    EXPECT_EQ(queue.size(), 0);

    queue.push(1);
    EXPECT_EQ(queue.size(), 1);

    auto value = queue.extract();
    EXPECT_EQ(queue.size(), 0);
    EXPECT_EQ(value, 1);
}

TEST(queue_test, push_extract_no_copy)
{
    thread_safe_queue<std::unique_ptr<int>> queue;
    EXPECT_EQ(queue.size(), 0);

    queue.push(std::make_unique<int>(42));
    EXPECT_EQ(queue.size(), 1);

    auto unique_ptr = queue.extract();
    EXPECT_EQ(queue.size(), 0);
    EXPECT_EQ(*unique_ptr, 42);
}
