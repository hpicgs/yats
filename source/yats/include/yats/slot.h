#pragma once

#include <cstdint>
#include <tuple>
#include <type_traits>

namespace yats
{

template <typename... Args>
using output_bundle = std::tuple<Args...>;

/// <summary>
/// <para>Main class to represent an slot</para>
/// <para><c>T</c> Type of slot value</para>
/// <para><c>Id</c> Unique identifier of slot</para>
/// </summary>
template<typename T, uint64_t Id>

class slot
{
public:
    using value_type = T;
    static constexpr uint64_t id = Id;

    static_assert(std::is_move_constructible_v<value_type>, "The slots value type must be move constructible.");

    /// <summary>Creates a new slot object.</summary>
    /// <param name = "value">Initial value of slot</param>
    slot(value_type value)
        : m_value{ std::move(value) }
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
