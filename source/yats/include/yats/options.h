#pragma once

#include <functional>
#include <map>
#include <memory>
#include <mutex>
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
    typed_member(Type Task::*member_pointer)
        : abstract_member<Task>(typeid(Type))
        , pointer(member_pointer)
    {
    }

    Type Task::*pointer;
};

template <typename Task>
struct member_pointer
{
    template <typename Type>
    member_pointer(Type Task::*pointer)
        : member(std::make_unique<typed_member<Task, Type>>(pointer))
    {
    }

    std::shared_ptr<abstract_member<Task>> member;
};

template <typename Task>
using options_map = std::map<std::string, member_pointer<Task>>;

template <typename Task>
class option_storage
{
public:
    option_storage(options_map<Task> option)
        : m_values(std::move(option))
    {
    }

    template <typename Type>
    void update(const std::string& key, Type value)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto option = m_values.find(key);
        if (option == m_values.end())
        {
            throw std::logic_error("key: '" + key + "' not found.");
        }

        auto member = option->second.member.get();
        if (member->type != std::type_index(typeid(Type)))
        {
            throw std::logic_error("key: '" + key + "' has an incompatible type.");
        }

        auto typed_member_pointer = static_cast<typed_member<Task, Type>*>(member)->pointer;
        m_changes.push_back([typed_member_pointer, value = std::move(value)](Task* task) mutable {
            task->*typed_member_pointer = std::move(value);
        });
    }

    void make_updates_visible(Task* task)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        for (auto& change : m_changes)
        {
            change(task);
        }
        m_changes.clear();
    }

protected:
    options_map<Task> m_values;
    std::vector<std::function<void(Task*)>> m_changes;
    std::mutex m_mutex;
};

template <typename Task>
using options_ptr = std::unique_ptr<option_storage<Task>>;

template <typename T>
struct has_static_options
{
    template <typename U>
    static auto test_function(int) -> decltype(U::options());
    template <typename U>
    static std::false_type test_function(...);

    static constexpr bool value = std::is_same<decltype(test_function<T>(0)), options_map<T>>::value;
};

template <typename T>
static constexpr bool has_static_options_v = has_static_options<T>::value;

template <typename T>
struct has_options
{
    template <typename U>
    static char test_function(decltype(&U::options));
    template <typename U>
    static int test_function(...);
    static constexpr bool value = sizeof(test_function<T>(0)) == sizeof(char);
};

template <typename T>
static constexpr bool has_options_v = has_options<T>::value;
}
