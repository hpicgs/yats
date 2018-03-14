#pragma once

#include <set>
#include <string>

namespace yats
{

class thread_group_helper;

class thread_group
{
public:
    explicit thread_group(const std::string& name)
        : m_names{ name }
    {
    }

    static thread_group main_thread()
    {
        return thread_group(name_for(MAIN));
    }

    static thread_group any_thread()
    {
        return thread_group(name_for(ANY));
    }

    const std::set<std::string>& names() const
    {
        return m_names;
    }

    thread_group& operator|=(const thread_group& other)
    {
        m_names.insert(other.m_names.cbegin(), other.m_names.cend());

        // As soon as we have a thread constraint we no longer want the any thread constraint
        m_names.erase(name_for(ANY));

        return *this;
    }

    friend thread_group operator|(thread_group lhs, const thread_group& rhs)
    {
        lhs |= rhs;
        return lhs;
    }

    // COUNT is the number of given_groups we currently have
    enum given_groups : size_t
    {
        ANY,
        MAIN,
        COUNT
    };

    static std::string name_for(given_groups group)
    {
        switch (group)
        {
        case ANY:
        {
            return "";
        }
        case MAIN:
        {
            return "main";
        }
        default:
        {
            throw std::logic_error("The implementation of yats is broken.");
        }
        }
    }

protected:
    std::set<std::string> m_names;
};

template <typename T>
struct has_thread_constraints
{
    template <typename U>
    static auto test_function(int) -> decltype(U::thread_constraints());
    template <typename U>
    static std::false_type test_function(...);

    static constexpr bool value = std::is_same<decltype(test_function<T>(0)), yats::thread_group>::value;
};

template <typename T>
static constexpr bool has_thread_constraints_v = has_thread_constraints<T>::value;
}
