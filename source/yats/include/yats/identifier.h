#pragma once
#include <algorithm>
#include <cstdint>
#include <string>

namespace yats
{
/**
 * Static class to encapsulate the functionality of hashing a string to a 64-bit numeric value.
 * Only the first 12 characters of the string will be used for hashing.
 * In the process of hashing information about case-sensitivity is lost.
 * All characters besides [a-z], [A-Z], ' ', '*', '/', '&', '#' will be treated as character '_'. 
 */
class identifier
{
public:
    /**
    * Converts {@code character} to an unsigned int.
    * Supported characters are [a-z],[A-Z], ' ', '*', '/', '&', '#' and '_'.
    * Other characters wil be treated like '_'. Characters [a-z] and [A-Z]
    * are represented by the same unsigned int.
    * @param character Character to convert
    * @return Numeric representation of character as 64-bit unsigned int-
    */
    static constexpr uint64_t char_to_int(char character)
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

        switch (character)
        {
        case ' ':
            return special_character_begin + 0;
        case '*':
            return special_character_begin + 1;
        case '/':
            return special_character_begin + 2;
        case '&':
            return special_character_begin + 3;
        case '#':
            return special_character_begin + 4;
        default:
            return special_character_begin + 5;
        }
    }

    /**
     * Converts {@code string} to a 64-bit numeric value.
     * @param string Pointer to the string to convert
     * @param length Length of {@code string}. If length is greater than 12 the string wil be truncated to the first 12
     * characters.
     * @return 64-bit id value
     */
    static constexpr uint64_t string_to_id(const char* string, size_t length)
    {
        if (length == 0)
            return 0;

        // The length of a string is limited to 12 characters excluding the termination character.
        constexpr auto max_length = 12;
        const auto truncated_length = std::min(static_cast<int>(length), max_length);

        auto value = char_to_int(string[truncated_length - 1]);

        // Highest 60 bits encode the string with up to 12 characters from an 32 character alphabet.
        // Highest 5 bits contain last character in string.
        for (auto i = truncated_length - 2; i >= 0; --i)
        {
            value = (value << width_of_character) | char_to_int(string[i]);
        }

        // Lowest 4 bits encode the length of the string.
        value = (value << width_of_length) | truncated_length;

        return value;
    }

    /**
     * Converts {@code id} to a string. This function is the inverse to {@link identifier#string_to_id(const char*, size_t) string_to_id}.
     * <p>
     * Note: Keep in mind that the hashing process is not lossless.
     * </p>
     * @param id Id to convert to a string.
     * @return String
     */
    static std::string id_to_string(uint64_t id)
    {
        std::string s;

        // lowest 4 bits = length;
        const int length = id & 0b1111;
        // cut off length (last 4 bits)
        id = id >> width_of_length;

        s.reserve(length);

        const uint64_t mask = 0b11111;

        for (auto i = 0; i < length; ++i)
        {
            const auto char_as_int = id & mask;
            s.push_back(int_to_char(char_as_int));
            id = id >> width_of_character;
        }
        return s;
    }

private:
    /**
     * Prevents class from being instantiated.
     */
    identifier() = default;

    // Defines a protocol to convert a string to an id.
    // Size of a single character in bits
    static const size_t width_of_character = 5;
    // Number of bits used to encode the string length.
    static const size_t width_of_length = 4;
    /* The alphabet only holds 32 characters. Supported characters are
     * [A-Z], ' ', '*', '/', '&', '#' and '_'.
     * special_character_begin marks the first special caharcter after 'Z'
     */   
    static const size_t special_character_begin = 26;

    /**
     * Converts an unsigned int to a character.
     * This method is the inverse to {@link identifier#char_to_int(char) char_to_int}.
     * @param character_as_int Integer representation of character to convert
     * @return Character to {@code character}
     */
    static constexpr char int_to_char(uint64_t character_as_int)
    {
        // Upper case
        if (character_as_int < special_character_begin)
        {
            return static_cast<char>(character_as_int + 'A');
        }

        switch (character_as_int)
        {
        case special_character_begin + 0:
            return ' ';
        case special_character_begin + 1:
            return '*';
        case special_character_begin + 2:
            return '/';
        case special_character_begin + 3:
            return '&';
        case special_character_begin + 4:
            return '#';
        default:
            return '_';
        }
    }
};
}

/**
 * Generates a 64-bit id from {@code string}.
 * @param string Pointer to a string.
 * @return Id to {@code string}.
 */
constexpr uint64_t id(const char* string)
{
    size_t length = 0;
    while (string[length])
    {
        ++length;
    }

    return yats::identifier::string_to_id(string, length);
}

/**
 * Operator generate an id to the sequence '"some_string"_id'.
 * @param string String to generate an id from
 * @param length Length of string
 * @return Id
 */
constexpr uint64_t operator"" _id(const char* string, size_t length)
{
    return yats::identifier::string_to_id(string, length);
}
