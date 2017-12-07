#include <gmock/gmock.h>


#include <yats/Identifier.h>

template <uint64_t>
void is_compiletime_constant()
{
}

TEST(identifier_test, is_compiletime_constant)
{
	using namespace yats;
	is_compiletime_constant<"value"_id>();
	is_compiletime_constant<yats::id("value")>();
}

TEST(identifier_test, different_syntax_is_same)
{
	using namespace yats;
	EXPECT_EQ("value"_id, yats::id("value"));
}

TEST(identifier_test, supports_full_range)
{
	// Test if every possible character returns a valid output.
	for (int i = std::numeric_limits<char>::min(); i <= std::numeric_limits<char>::max(); ++i)
	{
		EXPECT_LT(yats::lookup(static_cast<char>(i)), 32);
	}
}

TEST(identifier_test, is_case_insensitive)
{
	EXPECT_EQ(yats::id("value"), yats::id("Value"));
	EXPECT_EQ(yats::id("value"), yats::id("VALUE"));
	EXPECT_EQ(yats::id("valUE"), yats::id("vALUE"));
}

TEST(identifier_test, ignores_characters_after_max_length)
{
	EXPECT_EQ(yats::id("123456789012"), yats::id("1234567890123"));

	EXPECT_EQ(yats::id("1234567890124"), yats::id("1234567890123"));
}

TEST(identifier_test, handles_special_characters)
{
	// Every special character that is not supported gets converted to '_'.
	EXPECT_EQ(yats::id("123#()[]"), yats::id("___#____"));
}
