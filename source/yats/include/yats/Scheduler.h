#pragma once

#include <string>
#include <vector>

#include "TaskConfigurator.h"

namespace yats
{

class Scheduler
{
public:
	explicit Scheduler(const std::map<std::string, AbstractTaskConfigurator>& task_configurators)
	{

	}

	void run()
	{
		// TODO(anyone): Initial iteration to find first task to run
		auto* first_task = new Nodecontainer();
		schedule(first_task);

		// Run through entire pipeline
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
	bool can_run(AbstractNodecontainer* task_container)
	{
		return false;
	}

	void schedule(AbstractNodecontainer* task_container)
	{

	}

	AbstractNodecontainer* next_task()
	{
		return nullptr;
	}

	std::vector<AbstractNodecontainer*> following_tasks(AbstractNodecontainer* task_container)
	{
		return std::vector<AbstractNodecontainer*>();
	}
};

}  // namespace yats
