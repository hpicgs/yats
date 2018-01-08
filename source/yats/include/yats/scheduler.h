#pragma once

#include <algorithm>
#include <string>
#include <vector>
#include <iostream>

#include <yats/task_configurator.h>

namespace yats
{

class scheduler
{
public:
    explicit scheduler(const std::vector<std::unique_ptr<abstract_task_configurator>>& task_configurators)
        : m_tasks(abstract_task_configurator::build(task_configurators))
    {
    }

    void run()
    {
        std::vector<abstract_task_container*> to_run;
        for (auto& elem : m_tasks)
        {
            to_run.push_back(elem.get());
        }

        while (to_run.size() > 0)
        {
            auto runnable = std::find_if(to_run.begin(), to_run.end(), [](abstract_task_container* task) {
                return task->can_run();
            });

            (*runnable)->run();

			//auto f = (*runnable)->following_nodes();
			//std::cout << "Following: ";
			//for (auto e : f)
			//{
			//	std::cout << e << ", ";
			//}
			//std::cout << std::endl;

            to_run.erase(runnable);
        }
    }

protected:
    // Stores all task_containers with their position as an implicit id
    std::vector<std::unique_ptr<abstract_task_container>> m_tasks;
};
}
