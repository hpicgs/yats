#pragma once
#include <algorithm>
#include <cstdint>
#include <cstdlib>

namespace yats
{

//! This function transformes a character to a smaller alphabet.
constexpr int lookup(char character)
{
	// Upper case
	if (character >= 'A' && character <= 'Z')
	{
		return character - 'A';
	}

	// Lower case
	if (character >= 'a' && character <= 'z')
	{
		return character - 'a';
	}

	// Special characters
	constexpr int unique_begin = 26;
	switch (character)
	{
	case ' ':
		return unique_begin + 0;
	case '*':
		return unique_begin + 1;
	case '/':
		return unique_begin + 2;
	case '&':
		return unique_begin + 3;
	case '#':
		return unique_begin + 4;
	default:
	case '_':
		return unique_begin + 5;
	}
}

constexpr uint64_t name(const char *string, size_t length)
{
	uint64_t value = 0;

	// The length of a string is limited to 12 characters excluding the termination character.
	constexpr int max_length = 12;
	const int truncated_length = std::min(static_cast<int>(length), max_length);

	// Highest 60 bits encode the string with up to 12 characters from an 32 character alphabet defined by the lookup function.
	for (int i = max_length - 1; i >= 0; --i)
	{
		if (i <= truncated_length)
		{
			value = value | lookup(string[i]);
		}
		value = value << 5;
	}

	// Lowest 4 bits encode the length of the string.
	value = value | truncated_length;

	return value;
}

constexpr uint64_t id(const char *string)
{
	size_t length = 0;
	while (string[length])
	{
		++length;
	}

	return name(string, length);
}

constexpr uint64_t operator "" _id(const char *string, size_t length)
{
	return name(string, length);
}

}  // namespace yats
