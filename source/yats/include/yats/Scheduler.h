#pragma once

#include <string>
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
