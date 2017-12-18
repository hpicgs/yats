#pragma once

#include <map>
#include <memory>
#include <string>
#include <vector>

#include <yats/TaskConfigurator.h>
#include <yats/Util.h>

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
		static_assert(has_unique_ids_v<typename decltype(MakeHelper(&Task::run))::ReturnType>, "Can not add Task because multiple Outputs share the same Id.");

		m_tasks[name] = std::make_unique<TaskConfigurator<Task>>();
		return static_cast<TaskConfigurator<Task>*>(m_tasks[name].get());
	}

	void run()
	{
		std::vector<std::unique_ptr<AbstractTaskContainer>> tasks;
		for (auto &elem : m_tasks)
		{
			tasks.emplace_back(elem.second->make());
		}
	}

private:

	std::map<std::string, std::unique_ptr<AbstractTaskConfigurator>> m_tasks;
};

}  // namespace yats
