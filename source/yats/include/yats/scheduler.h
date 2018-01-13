#pragma once

#include <algorithm>
#include <string>
#include <vector>
#include <iostream>

#include <yats/pipeline.h>
#include <yats/task_configurator.h>

namespace yats
{

class scheduler
{
public:
    explicit scheduler(const pipeline& pipeline)
        : m_tasks(pipeline.build())
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

            to_run.erase(runnable);
        }
    }

protected:
    // Stores all task_containers with their position as an implicit id
    std::vector<std::unique_ptr<abstract_task_container>> m_tasks;
};
}
