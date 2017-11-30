#pragma once

template<typename T, size_t Id>
class Input
{
public:
	using value_type = T;
	static constexpr size_t ID = Id;

	Input(T value) : m_value{value} {}

	operator const T&() const { return m_value; }
	operator T&() { return m_value; }
	//T& operator*() { return m_value; }
protected:
	T m_value;
private:
		
};
