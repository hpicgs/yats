
#include <gmock/gmock.h>


#include <yats/Identifier.h>

template<uint64_t> 
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
		EXPECT_LT(lookup(static_cast<char>(i)), 32);
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
