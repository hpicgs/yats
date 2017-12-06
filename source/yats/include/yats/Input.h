#pragma once


/// <summary>
/// <para>Main class to represent an input</para>
/// <para><c>T</c> Type of input value</para>
/// <para><c>Id</c> Unique identifier of input</para>
/// </summary>
template<typename T, size_t Id>
class Input
{
public:
	using value_type = T;
	static constexpr size_t ID = Id;

	/// <summary>Creates a new Input object.</summary>
	/// <param name = "value">Initial value of input</param>
	Input(T value) : m_value{value} {}

	operator const T&() const { return m_value; }
	operator T&() { return m_value; }
	//T& operator*() { return m_value; }
protected:
	T m_value;
private:
};

