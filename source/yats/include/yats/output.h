#pragma once

namespace yats
{

template <typename... Args>
using output_bundle = std::tuple<Args...>;

/// <summary>
/// <para>Main class to represent an output</para>
/// <para><c>T</c> Type of output value</para>
/// <para><c>Id</c> Unique identifier of output</para>
/// </summary>
template <typename T, uint64_t Id>
class output
{
public:
    using value_type = T;
    static constexpr uint64_t id = Id;

    /// <summary>Creates a new Output object.</summary>
    /// <param name = "value">Initial value of output</param>
    output(value_type value)
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

protected:
    value_type m_value;
};

template <typename T, uint64_t Id>
constexpr uint64_t output<T, Id>::id;
}
