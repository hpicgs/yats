#pragma once

#include <functional>
#include <map>
#include <memory>
#include <string>
#include <typeindex>
#include <vector>

namespace yats
{

template <typename Task>
struct abstract_member
{
    abstract_member(const std::type_info& info)
        : type(info)
    {
    }
    virtual ~abstract_member() = default;

    std::type_index type;
};

template <typename Task, typename Type>
struct typed_member : public abstract_member<Task>
{
    typed_member(Type Task::* member_pointer)
        : abstract_member(typeid(Type))
        , pointer(member_pointer)
    {
    }

    Type Task::* pointer;
};

template <typename Task>
class member_pointer
{
public:
    template <typename Type>
    member_pointer(Type Task::* pointer)
        : member(std::make_unique<typed_member<Task, Type>>(pointer))
    {
    }

protected:
    std::unique_ptr<abstract_member<Task>> member;
};

template <typename Task>
using options_map = std::map<std::string, member_pointer<Task>>;

template <typename Task>
class options
{
public:
    options(options_map<Task> option)
        : m_values(std::move(option))
    {
    }

    template <typename Type>
    void update(const std::string &key, Type value)
    {
        auto option = m_values.find(key);
        if (option == m_values.end())
        {
            throw std::logic_error("key: '" + key + "' not found.");
        }

        auto member_pointer = option.second.member.get();
        if (member_pointer->type != std::type_index(typeid(Type)))
        {
            throw std::logic_error("key: '" + key + "' has an incompatible type.");
        }

        auto typed_member_pointer = static_cast<member<Task, Type>>(member_pointer).pointer;
        m_changes.push_back([typed_member_pointer, value = std::move(value)](Task *task) mutable
        {
            task.*typed_member_pointer = std::move(value);
        });
    }

    void make_updates_visible(Task *task)
    {
        for (auto &change : m_changes)
        {
            change(task);
        }
        m_changes.clear();
    }

protected:
    options_map<Task> m_values;
    std::vector<std::function<void(Task*)>> m_changes;
};

}
