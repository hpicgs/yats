#pragma once

#include <functional>


class AbstractFunctional
{
};

template <typename Parameter>
class Functional : public AbstractFunctional
{
public:
	Functional(std::function<void(Parameter)> &function)
		: m_function(function)
	{
	}

	auto func()
	{
		return m_function;
	}

protected:
	std::function<void(Parameter)> m_function;
};
