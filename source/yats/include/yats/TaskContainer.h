#pragma once

#include <tuple>

#include <yats/Util.h>

namespace yats
{

class AbstractTaskContainer
{
public:
	AbstractTaskContainer() = default;

	virtual ~AbstractTaskContainer() = default;

	virtual bool canRun() const = 0;

private:


};


template <typename Task>
class TaskContainer : public AbstractTaskContainer
{
public:

	using Helper = decltype(MakeHelper(&Task::run));

	TaskContainer()
		: m_current(0)
	{
	}

	bool canRun() const override
	{
		return m_current == Helper::ParameterCount;
	}

private:

	typename Helper::Input m_parameter;
	Task m_task;
	int m_current;
};

} // namespace yats
