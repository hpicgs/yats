#pragma once

#include <string>
#include <sstream>
#include <vector>

namespace yats
{

class class_name
{
public:
    template <typename T>
    static std::string get()
    {
#ifdef _MSC_VER
        return get_class_name_msvc<T>();
#elif __clang__
        return get_class_name_clang<T>();
#elif __GNUC__
        return get_class_name_gcc<T>();
#else
        static_assert(false, "unsupported compiler.");
#endif
    }

protected:
#ifdef __clang__
    template <typename T>
    static std::string get_class_name_clang()
    {
        std::string name(__PRETTY_FUNCTION__);
        const std::string start_token("[T = ");
        const auto start = name.find(start_token);
        const auto end = name.find(']', start);
        auto substr = name.substr(start + start_token.length(), end - start - start_token.length());
        return get_class_name(substr, 0, substr.length() - 1);
    }
#elif __GNUC__
    template <typename T>
    static std::string get_class_name_gcc()
    {
        std::string name(__PRETTY_FUNCTION__);
        const std::string start_token("[with T = ");
        const auto start = name.find(start_token);
        const auto end = name.find(';', start);
        auto substr = name.substr(start + start_token.length(), end - start - start_token.length());
        return get_class_name(substr, 0, substr.length() - 1);
    }
#elif _MSC_VER
    template <typename T>
    static std::string get_class_name_msvc()
    {
        // start_token must match the function name + <
        const std::string start_token("get_class_name_msvc<");
        std::string name(__FUNCSIG__);
        const auto start = name.find(start_token);
        const auto end = name.find_last_of('>');

        return get_class_name(name, start + start_token.length(), end - 1);
    }
#endif

    static std::string get_class_name(const std::string& str, size_t start, size_t end)
    {
        const auto left = str.find_first_of('<', start);
        const auto right = str.find_last_of('>', end);

        std::string class_name;
        if (left < std::string::npos)
        {
            class_name = trim_left(str.substr(start, left - start), false);
        }

        if (left < right)
        {
            return class_name + '<' + get_class_name(str, left + 1, right - 1) + '>';
        }

        // Jetzt muss noch am Komma gesplittet werden.
        auto words = split(str.substr(start, end - start + 1));
        // jetzt muss links alles vom wort abgschnitten werden.
        for (size_t i = 0; i < words.size(); i++)
        {
            words[i] = trim_left(words[i], true);
#ifdef __GNUC__
            if (words[i][0] == ' ')
            {
                words[i] = words[i].substr(1, words[i].length() - 1);
            }
#endif
        }

        return join(words, ",");
    }

    static std::string trim_left(const std::string& str, const bool ignore_space)
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

    static std::string join(const std::vector<std::string> strings, const std::string& delimiter)
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