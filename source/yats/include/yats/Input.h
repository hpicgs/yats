#pragma once

namespace yats
{

/// <summary>
/// <para>Main class to represent an input</para>
/// <para><c>T</c> Type of input value</para>
/// <para><c>Id</c> Unique identifier of input</para>
/// </summary>
template<typename T, uint64_t Id>
class Input
{
public:
	using value_type = T;
	constexpr static uint64_t ID = Id;

	/// <summary>Creates a new Input object.</summary>
	/// <param name = "value">Initial value of input</param>
	Input(T value) : m_value{value} {}

	operator const T&() const { return m_value; }
	operator T&() { return m_value; }

protected:
	T m_value;
};

template<typename T, uint64_t Id>
constexpr uint64_t Input<T, Id>::ID;

}  // namespace yats
