#pragma once

#include <string>
#include <sstream>
#include <vector>

namespace yats
{
/**
 * Static class to determine the class name of class at runtime.
 */
class class_name
{
public:
    /**
     * Determines the class name {@code T}.
     * <p>
     * T: Class/struct whose name is to be determined.
     * </p>
     */
    template <typename T>
    static std::string get()
    {
        return name<T>();
    }

protected:
#ifdef __clang__
    template <typename T>
    static std::string name()
    {
        // example return value:
        // std::string get() [T = some_struct]
        std::string name(__PRETTY_FUNCTION__);
        
        const std::string start_token("[T = ");
        const auto start = name.find(start_token);
        const auto end = name.find(']', start);
        auto substr = name.substr(start + start_token.length(), end - start - start_token.length());
        return parse_class_name(substr, 0, substr.length() - 1);
    }
#elif __GNUC__
    template <typename T>
    static std::string name()
    {
        // example return value:
        // std::__cxx11::string get() [with T = main()::some_struct; std::__cxx11::string = std::__cxx11::basic_string<char>]
        std::string name(__PRETTY_FUNCTION__);
        
        const std::string start_token("[with T = ");
        const auto start = name.find(start_token);
        const auto end = name.find(';', start);
        auto substr = name.substr(start + start_token.length(), end - start - start_token.length());
        return parse_class_name(substr, 0, substr.length() - 1);
    }
#elif _MSC_VER
    template <typename T>
    static std::string name()
    {
        // example return value:
        // class std::basic_string<char,struct std::char_traits<char>,class std::allocator<char> > __cdecl yats::get_class_name<struct util_test_get_class_name_test_Test::TestBody::some_struct>(void)
        std::string name(__FUNCSIG__);

        // start_token must match the function name + "<"
        const std::string start_token("name<");
        const auto start = name.find(start_token);
        const auto end = name.find_last_of('>');
        return parse_class_name(name, start + start_token.length(), end - 1);
    }
#else
    template <typename T>
    static std::string name()
    {
        // Unit tests will fail.
        const std::string typeid_name = typeid(T).name();
        std::string name;

        for (const auto& c : typeid_name)
        {
            switch (c)
            {
            case '<':
                name.append("&lt;");
                break;
            case '>':
                name.append("&gt;");
                break;
            case '&':
                name.append("&amp;");
                break;
            case '"':
                name.append("&quot;");
                break;
            case '\'':
                name.append("&apos;");
                break;
            default:
                name.append(1, c);
            }
        }

        return name;
    }
#endif

    /**
     * Parses and constructs the class name contained in {@code str[start, end]}.
     * @param str Reference to string to be parsed
     * @param start Lower bound of {@code str} to look at.
     * @param end Upper bound of {@code str} to look at.
     * @return Class name parsed from {@code str}
     */
    static std::string parse_class_name(const std::string& str, size_t start, size_t end)
    {
        const auto left = str.find_first_of('<', start);
        const auto right = str.find_last_of('>', end);

        std::string class_name;
        if (left < std::string::npos)
        {
            // read CLASS_NAME from CLASS_NAME<TYPE1, TYPE2,...>
            class_name = trim_left(str.substr(start, left - start), false);
        }

        if (left < right)
        {
            // Extract types in class_name<TYPE1,TYPE2,...>
            return class_name + "&lt;" + parse_class_name(str, left + 1, right - 1) + "&gt;";
        }

        // Isolate types in TYPE1, TYPE2,...
        auto words = split(str.substr(start, end - start + 1));
        
        // remove additional identifiers before a type name id1::id2::id3::TYPE1
        for (size_t i = 0; i < words.size(); i++)
        {
            words[i] = trim_left(words[i], true);
#ifdef __GNUC__
            // GCC puts a space between type enumeration class_name<TYPE1, TYPE2, TYPE3, ...>
            if (words[i][0] == ' ')
            {
                words[i] = words[i].substr(1, words[i].length() - 1);
            }
#endif
        }

        // Concatenate types (where additional identifiers have been removed) with a ","
        return join(words, ",");
    }

    /**
     * Removes everything before (including) the last "::" or " ".
     * @param str Reference to string to trim.
     * @param ignore_space If false, everything before the last space (including the space) will be trimmed.
     * @return Trimmed string
     */
    static std::string trim_left(const std::string& str, bool ignore_space)
    {
        auto start = str.find_last_of("::");
        if (start == std::string::npos && !ignore_space)
        {
            start = str.find_last_of(' ');
        }

        if (start != std::string::npos)
        {
            return str.substr(start + 1);
        }
        return str;
    }

    /**
     * Splits {@code str} at every ",".
     *  @param str String to split.
     *  @return Vector of strings
     */
    static std::vector<std::string> split(const std::string& str)
    {
        std::vector<std::string> words;
        std::string s;
        std::istringstream stream(str);
        while (std::getline(stream, s, ','))
        {
            words.push_back(s);
        }
        return words;
    }

    /**
     * Concatenates all strings in {@code string} with {@code delimiter} as seperator.
     * @param strings Vector of string
     * @param delimiter Delimiter to concatenate strings with
     * @return Concatenated string
     */
    static std::string join(const std::vector<std::string>& strings, const std::string& delimiter)
    {
        std::string str;

        for (const auto& string : strings)
        {
            str.append(string);
            str.append(delimiter);
        }

        if (!str.empty())
        {
            str.pop_back();
        }

        return str;
    }
};
}
