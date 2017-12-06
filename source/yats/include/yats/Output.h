
#pragma once


template<typename... Args>
using OutputBundle = std::tuple<Args...>;

/// <summary>
/// <para>Main class to represent an output</para>
/// <para><c>T</c> Type of output value</para>
/// <para><c>Id</c> Unique identifier of output</para>
/// </summary>
template<typename T, size_t Id>
class Output
{
public:
	using value_type = T;
	static constexpr size_t ID = Id;

	/// <summary>Creates a new Output object.</summary>
	/// <param name = "value">Initial value of output</param>
	Output(T value) : m_value{ value } {}

	operator const T&() const { return m_value; }
	operator T&() { return m_value; }

protected:
	T m_value;
};
