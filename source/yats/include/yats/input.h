#pragma once

namespace yats
{

/// <summary>
/// <para>Main class to represent an input</para>
/// <para><c>T</c> Type of input value</para>
/// <para><c>Id</c> Unique identifier of input</para>
/// </summary>
template <typename T, uint64_t Id>
class input
{
public:
    using value_type = T;
    static constexpr uint64_t id = Id;

    /// <summary>Creates a new Input object.</summary>
    /// <param name = "value">Initial value of input</param>
    input(value_type value)
        : m_value{ value }
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

protected:
    value_type m_value;
};

template <typename T, uint64_t Id>
constexpr uint64_t input<T, Id>::id;
}
