#pragma once

#include <string>
#include <sstream>
#include <vector>

namespace yats
{
class msvc_class_name_parser
{
public:
    template <typename T>
    static std::string get_class_name()
    {
        std::string name(__FUNCSIG__);
        const auto start = name.find("get_class_name<");
        const auto end = name.find_last_of('>');

        return get_class_name(name, start + 15, end - 1);
    }

protected:
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

template <typename T>
std::string get_class_name()
{
#ifdef _MSC_VER
    return msvc_class_name_parser::get_class_name<T>();



#elif __GNUC__
#else
    static_assert(false, "unsupported compiler.");
#endif
}
}