#pragma once

#include <memory>
#include <string>
#include <vector>

#include <yats/TaskConfigurator.h>
#include <yats/Util.h>
#include <yats/Scheduler.h>

namespace yats
{

class Pipeline
{
public:
    Pipeline() = default;

	template <typename Task>
	TaskConfigurator<Task>* add()
	{
		static_assert(has_unique_ids_v<typename decltype(MakeHelper(&Task::run))::WrappedInput>, "Can not add Task because multiple Inputs share the same Id.");
		static_assert(has_unique_ids_v<typename decltype(MakeHelper(&Task::run))::ReturnBase>, "Can not add Task because multiple Outputs share the same Id.");

		m_tasks.push_back(std::make_unique<TaskConfigurator<Task>>());
		return static_cast<TaskConfigurator<Task>*>(m_tasks.back().get());
	}

	void run()
	{
		auto tasks = AbstractTaskConfigurator::build(m_tasks);
	}

	Scheduler build()
	{
		return Scheduler{m_tasks};
	}

protected:
	std::vector<std::unique_ptr<AbstractTaskConfigurator>> m_tasks;
};
}
