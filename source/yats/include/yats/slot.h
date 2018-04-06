#pragma once

#include <cstdint>
#include <tuple>
#include <type_traits>
#include <utility>

namespace yats
{

template <typename... Args>
class output_bundle
{
public:
    template <typename... Params>
    output_bundle(Params&&... params)
        : m_return_values(std::forward<Params>(params)...)
    {
    }

    std::tuple<Args...> tuple() &&
    {
        return std::move(m_return_values);
    }

protected:
    std::tuple<Args...> m_return_values;
};

/**
 * Main class to represent a slot.
 * @param T Type of slot value
 * @param Id Unique identifier of slot.
 */
template <typename T, uint64_t Id>
class slot
{
public:
    using value_type = T;
    static constexpr uint64_t id = Id;

    static_assert(std::is_move_constructible<value_type>::value, "The slots value type has to be move constructible.");

    /**
     * Creates a new slot object.
     * @param value Initial value of slot
     */
    slot(value_type value)
        : m_value{ std::move(value) }
    {
    }

    template <uint64_t OtherID>
    slot(const slot<T, OtherID>& other)
        : m_value(other.m_value)
    {
    }

    template <uint64_t OtherID>
    slot(slot<T, OtherID>&& other)
        : m_value(std::move(other.m_value))
    {
    }

    operator const value_type&() const
    {
        return m_value;
    }

    operator value_type&()
    {
        return m_value;
    }

    T& operator*()
    {
        return m_value;
    }

    template <typename Type = T>
    std::enable_if_t<std::is_pointer<Type>::value, Type> operator->()
    {
        return m_value;
    }

    template <typename Type = T>
    std::enable_if_t<!std::is_pointer<Type>::value, Type*> operator->()
    {
        return &m_value;
    }

    template <typename Type = T>
    std::enable_if_t<std::is_copy_constructible<Type>::value, Type> clone() const
    {
        return m_value;
    }

    T extract()
    {
        return std::move(m_value);
    }

protected:
    value_type m_value;
};

template <typename T, uint64_t Id>
constexpr uint64_t slot<T, Id>::id;
}
