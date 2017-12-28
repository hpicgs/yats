#pragma once

#include <algorithm>
#include <string>
#include <vector>

#include <yats/TaskConfigurator.h>

namespace yats
{

class Scheduler
{
public:
	explicit Scheduler(const std::vector<std::unique_ptr<AbstractTaskConfigurator>>& task_configurators)
		: m_tasks(AbstractTaskConfigurator::build(task_configurators))
	{
		//for (const auto& task : m_tasks)
		//{
		//	if (task->can_run())
		//	{
		//		schedule(task.get());
		//	}
		//}
	}

	void run()
	{
		std::vector<AbstractTaskContainer*> to_run;
		for (auto &elem : m_tasks)
		{
			to_run.push_back(elem.get());
		}

		while (to_run.size() > 0)
		{
			auto runnable = std::find_if(to_run.begin(), to_run.end(), [](AbstractTaskContainer* task)
			{
				return task->can_run();
			});

			(*runnable)->run();
			to_run.erase(runnable);
		}
	}

protected:
	//void schedule(AbstractTaskContainer* task_container)
	//{
	//	m_scheduled.push(task_container);
	//}

	//AbstractTaskContainer* next_task()
	//{
	//	if (m_scheduled.empty())
	//	{
	//		return nullptr;
	//	}

	//	auto next = m_scheduled.front();
	//	m_scheduled.pop();
	//	return next;
	//}

	//std::vector<AbstractTaskContainer*> following_tasks(AbstractTaskContainer* /*task_container*/)
	//{
	//	std::vector<AbstractTaskContainer*> followers;
	//	followers.reserve(m_tasks.size());
	//	for (auto& task : m_tasks)
	//	{
	//		followers.push_back(task.get());
	//	}
	//	return followers;
	//}

	// Stores all TaskContainers with their position as an implicit id
	std::vector<std::unique_ptr<AbstractTaskContainer>> m_tasks;

	// Queue of all scheduled tasks
	//std::queue<AbstractTaskContainer*> m_scheduled;
};

}  // namespace yats
