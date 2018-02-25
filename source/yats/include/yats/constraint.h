#pragma once

#include <set>
#include <string>
#include <vector>

namespace yats
{

class thread_group
{
public:
    explicit thread_group(const std::string& name)
        : m_names{ name }
    {
    }

    static thread_group main_thread()
    {
        return thread_group(main_thread_name());
    }

    static const std::string& main_thread_name()
    {
        const static std::string name = "main";
        return name;
    }

    static size_t main_thread_number()
    {
        return 1;
    }

    static thread_group any_thread()
    {
        return thread_group(any_thread_name());
    }

    static const std::string& any_thread_name()
    {
        const static std::string name = "";
        return name;
    }

    static size_t any_thread_number()
    {
        return 0;
    }

    const std::vector<std::string>& names() const
    {
        return m_names;
    }

    thread_group& operator|=(const thread_group& other)
    {
        auto end_it = m_names.end();
        // As soon as we have a thread constraint we no longer want the any thread constraint
        if (m_names[0] == any_thread_name())
        {
            --end_it;
        }
        m_names.insert(end_it, other.m_names.cbegin(), other.m_names.cend());
        return *this;
    }

    friend thread_group operator|(thread_group lhs, const thread_group& rhs)
    {
        lhs |= rhs;
        return lhs;
    }

protected:
    std::vector<std::string> m_names;
};
}
