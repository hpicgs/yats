#pragma once

#include <string>
#include <vector>

#include <yats/TaskConfigurator.h>

namespace yats
{

class Scheduler
{
public:
	explicit Scheduler(const std::map<std::string, std::unique_ptr<AbstractTaskConfigurator>>& task_configurators)
	{

	}

	void run()
	{
		// TODO(anyone): Do initial run to find first runnable task
		AbstractTaskContainer* first_task = nullptr;
		schedule(first_task);

		while (auto* task = next_task())
		{
			task->run();
			for (auto* follower : following_tasks(task))
			{
				if (can_run(follower))
				{
					schedule(follower);
				}
			}
		}
	}

protected:
	bool can_run(AbstractTaskContainer* task_container)
	{
		return false;
	}

	void schedule(AbstractTaskContainer* task_container)
	{

	}

	AbstractTaskContainer* next_task()
	{
		return nullptr;
	}

	std::vector<AbstractTaskContainer*> following_tasks(AbstractTaskContainer* task_container)
	{
		return std::vector<AbstractTaskContainer*>();
	}
};

}  // namespace yats
