#pragma once

#include <map>
#include <memory>
#include <string>
#include <vector>

#include <yats/TaskConfigurator.h>

namespace yats
{

class Pipeline
{
public:

	Pipeline() = default;

	template <typename Task>
	TaskConfigurator<Task>* add(const std::string &name)
	{
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
