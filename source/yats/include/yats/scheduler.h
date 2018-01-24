#pragma once

#include <algorithm>
#include <string>
#include <vector>
#include <iostream>
#include <thread>
#include <mutex>

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

        std::function<void()> check_runnable;

        check_runnable = [&to_run, &check_runnable, this]()
        {
            std::lock_guard<std::mutex> lock(m_mutex);

            auto runnable = std::find_if(to_run.begin(), to_run.end(), [](abstract_task_container* task) {
                return task->can_run();
            });

            auto task = *runnable;
            to_run.erase(runnable);

            m_threads.emplace_back([&to_run, &check_runnable, task]()
            {
                task->run();
                check_runnable();
            });
        };
    }

protected:
    // Stores all task_containers with their position as an implicit id
    std::vector<std::unique_ptr<abstract_task_container>> m_tasks;
    std::vector<std::thread> m_threads;
    std::mutex m_mutex;
};
}
