#include <gmock/gmock.h>

#include <yats/identifier.h>

using namespace yats;

template <uint64_t>
void is_compiletime_constant()
{
}

TEST(identifier_test, is_compiletime_constant)
{
    is_compiletime_constant<"value"_id>();
    is_compiletime_constant<id("value")>();
}

TEST(identifier_test, different_syntax_is_same)
{
    EXPECT_EQ("value"_id, id("value"));
}

TEST(identifier_test, supports_full_range)
{
    // Test if every possible character returns a valid output.
    for (int i = std::numeric_limits<char>::min(); i <= std::numeric_limits<char>::max(); ++i)
    {
        EXPECT_LT(identifier::char_to_int(static_cast<char>(i)), 32);
    }
}

TEST(identifier_test, is_case_insensitive)
{
    EXPECT_EQ(id("value"), id("Value"));
    EXPECT_EQ(id("value"), id("VALUE"));
    EXPECT_EQ(id("valUE"), id("vALUE"));
}

TEST(identifier_test, ignores_characters_after_max_length)
{
    EXPECT_EQ(id("123456789012"), id("1234567890123"));
    EXPECT_EQ(id("1234567890124"), id("1234567890123"));
}

TEST(identifier_test, handles_special_characters)
{
    // Every special character that is not supported gets converted to '_'.
    EXPECT_EQ(id("123#()[]"), id("___#____"));
}

TEST(identifier_test, is_invertible_simple)
{
    EXPECT_EQ(identifier::id_to_string(id("")), "");
    EXPECT_EQ(identifier::id_to_string(id("y")), "Y");
    EXPECT_EQ(identifier::id_to_string(id("abc")), "ABC");
}

TEST(identifier_test, is_invertible_too_long)
{
    EXPECT_EQ(identifier::id_to_string(id("abcdefghijkl")), "ABCDEFGHIJKL");
    EXPECT_EQ(identifier::id_to_string(id("abcdefghijklmno")), "ABCDEFGHIJKL");
}

TEST(identifier_test, is_invertible_special_characters)
{
    EXPECT_EQ(identifier::id_to_string(id("ACPP WS17/18")), "ACPP WS__/__");
}
