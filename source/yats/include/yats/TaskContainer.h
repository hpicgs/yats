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

	virtual void run() = 0;

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

	void run() override
	{

	}

private:

	typename Helper::InputQueue m_input;
	typename Helper::ReturnCallbacks m_output;
	Task m_task;
	int m_current;
};

} // namespace yats
