#pragma once

#include <map>
#include <memory>
#include <string>
#include <vector>

#include <yats/TaskConfigurator.h>
#include <yats/Util.h>
#include "Scheduler.h"

namespace yats
{

class Pipeline
{
public:

	Pipeline() = default;

	template <typename Task>
	TaskConfigurator<Task>* add(const std::string &name)
	{
		static_assert(has_unique_ids_v<typename decltype(MakeHelper(&Task::run))::WrappedInput>, "Can not add Task because multiple Inputs share the same Id.");
		static_assert(has_unique_ids_v<typename decltype(MakeHelper(&Task::run))::ReturnBase>, "Can not add Task because multiple Outputs share the same Id.");

		m_tasks[name] = std::make_unique<TaskConfigurator<Task>>();
		return static_cast<TaskConfigurator<Task>*>(m_tasks[name].get());
	}

	void run()
	{
		auto tasks = AbstractTaskConfigurator::build(m_tasks);
	}

	Scheduler build()
	{
		return Scheduler{m_tasks};
	}

private:

	std::map<std::string, std::unique_ptr<AbstractTaskConfigurator>> m_tasks;
};

}  // namespace yats
