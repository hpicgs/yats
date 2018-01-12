#pragma once

#include <functional>

namespace yats
{

template <typename Return, typename... Parameters>
class lambda_task
{
public:

	Return run(Parameters... parameters)
	{
		return m_run(parameters);
	}

protected:
	std::function<Return(Parameters...)> m_run;
};
}
