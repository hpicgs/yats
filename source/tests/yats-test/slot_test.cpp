#include <gmock/gmock.h>

#include <memory>

#include <yats/slot.h>

using namespace yats;

TEST(slot_test, integral_value)
{
    slot<int, 0> integral(123);
    EXPECT_EQ(*integral, 123);
}

TEST(slot_test, integral_reference)
{
    slot<int, 0> integral(123);
    *integral += 1;
    EXPECT_EQ(*integral, 124);
}

TEST(slot_test, integral_cast)
{
    slot<int, 0> integral(123);
    int test = integral;
    EXPECT_EQ(test, 123);
}

TEST(slot_test, class_value)
{
    slot<std::string, 0> integral("123");
    EXPECT_EQ(*integral, "123");
}

TEST(slot_test, class_reference)
{
    slot<std::string, 0> integral("123");
    *integral += "4";
    EXPECT_EQ(*integral, "1234");

    integral->append("5");
    EXPECT_EQ(*integral, "12345");
}

TEST(slot_test, class_cast)
{
    slot<std::string, 0> integral("123");
    std::string test = integral;
    EXPECT_EQ(test, "123");
}

TEST(slot_test, pointer_value)
{
    std::string v("123");
    slot<std::string*, 0> integral(&v);
    EXPECT_EQ(**integral, "123");
}

TEST(slot_test, pointer_reference)
{
    std::string v("123");
    slot<std::string*, 0> integral(&v);
    **integral += "4";
    EXPECT_EQ(**integral, "1234");

    integral->append("5");
    EXPECT_EQ(**integral, "12345");
}

TEST(slot_test, pointer_cast)
{
    std::string v("123");
    slot<std::string*, 0> integral(&v);
    std::string* test = integral;
    EXPECT_EQ(*test, "123");
}

TEST(slot_test, perfect_forwarding_test)
{
    []() -> output_bundle<slot<std::unique_ptr<int>, 0>>
    {
        std::unique_ptr<int> pointer;
        return pointer;
    }();

    []() -> output_bundle<slot<std::unique_ptr<int>, 0>>
    {
        slot<std::unique_ptr<int>, 0> pointer(nullptr);
        return pointer;
    }();
}
