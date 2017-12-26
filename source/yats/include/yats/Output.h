#pragma once

namespace yats
{

template <typename... Args>
using OutputBundle = std::tuple<Args...>;

/// <summary>
/// <para>Main class to represent an output</para>
/// <para><c>T</c> Type of output value</para>
/// <para><c>Id</c> Unique identifier of output</para>
/// </summary>
template <typename T, uint64_t Id>
class Output
{
public:
    using value_type = T;
    constexpr static uint64_t ID = Id;

    /// <summary>Creates a new Output object.</summary>
    /// <param name = "value">Initial value of output</param>
    Output(value_type value)
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
constexpr uint64_t Output<T, Id>::ID;
}
