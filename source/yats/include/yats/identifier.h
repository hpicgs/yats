#pragma once
#include <algorithm>
#include <cstdint>
#include <string>

namespace yats
{
// Defines or protocol to convert a string to an id.
// Size of a single character in bits
const size_t width_of_character = 5;
// Number of bits used to encode the string length.
const size_t width_of_length = 4;

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

constexpr uint64_t name(const char* string, size_t length)
{
    if (length == 0)
        return 0;

    // The length of a string is limited to 12 characters excluding the termination character.
    constexpr int max_length = 12;
    const int truncated_length = std::min(static_cast<int>(length), max_length);

    uint64_t value = lookup(string[truncated_length - 1]);

    // Highest 60 bits encode the string with up to 12 characters from an 32 character alphabet defined by the lookup function.
    // Highest bit contains last character in string.
    for (int i = truncated_length - 2; i >= 0; --i)
    {
        value = (value << width_of_character) | lookup(string[i]);
    }

    // Lowest 4 bits encode the length of the string.
    value = (value << width_of_length) | truncated_length;

    return value;
}

static char int_to_char(uint64_t character)
{
    // Upper case
    if (character <= 25)
    {
        return static_cast<char>(character + 'A');
    }

    // Special characters
    constexpr auto unique_begin = 26;
    switch (character)
    {
    case unique_begin + 0:
        return ' ';
    case unique_begin + 1:
        return '*';
    case unique_begin + 2:
        return '/';
    case unique_begin + 3:
        return '&';
    case unique_begin + 4:
        return '#';
    default:
    case '_':
        return '_';
    }
}

static std::string id_to_string(uint64_t id)
{
    std::string s;

    // lowest 4 bits = length;
    const int length = id & 15;
    // cut off length (last 4 bits)
    id = id >> width_of_length;

    s.reserve(length);

    const auto mask = 0b11111;

    for (auto i = 0; i < length; ++i)
    {
        const uint64_t char_as_int = id & mask;
        s.push_back(int_to_char(char_as_int));
        id = id >> width_of_character;
    }
    return s;
}
}

constexpr uint64_t id(const char* string)
{
    size_t length = 0;
    while (string[length])
    {
        ++length;
    }

    return yats::name(string, length);
}

constexpr uint64_t operator"" _id(const char* string, size_t length)
{
    return yats::name(string, length);
}
