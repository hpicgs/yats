#pragma once

#include <algorithm>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include <yats/pipeline.h>
#include <yats/task_configurator.h>

namespace yats
{

class scheduler
{
public:
    explicit scheduler(pipeline pipeline)
        : m_tasks(pipeline.build([this](abstract_task_container* task) { task_received_input(task); }))
    {
    }

    scheduler(const scheduler& other) = delete;
    scheduler(scheduler&& other) = delete;

    ~scheduler() = default;

    scheduler& operator=(const scheduler& other) = delete;
    scheduler& operator=(scheduler&& other) = delete;

    void run()
    {
        std::vector<abstract_task_container*> to_run;
        for (auto& elem : m_tasks)
        {
            to_run.push_back(elem.get());
        }

        std::function<void()> check_runnable;

        check_runnable = [&to_run, &check_runnable, this]() {
            std::lock_guard<std::mutex> lock(m_mutex);

            while (true)
            {
                auto runnable = std::find_if(to_run.begin(), to_run.end(), [](abstract_task_container* task) {
                    return task->can_run();
                });

                if (runnable == to_run.end())
                {
                    return;
                }
                auto task = *runnable;
                to_run.erase(runnable);

                m_threads.emplace_back([&to_run, &check_runnable, task]() {
                    task->run();
                    check_runnable();
                });
            }
        };

        check_runnable();
        while (true)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            {
                std::lock_guard<std::mutex> lock(m_mutex);
                if (to_run.empty())
                {
                    break;
                }
            }
        }

        for (auto& thread : m_threads)
        {
            thread.join();
        }

        m_threads.clear();
    }

protected:
    void task_received_input(abstract_task_container* task)
    {
        (void) task;
        // TODO: the scheduler should check if this can run and schedule it
        // it is not implemented yet, because the current scheduler can not do this easily
        // and we change the scheduler right now anyway
    }

    // Stores all task_containers with their position as an implicit id
    std::vector<std::unique_ptr<abstract_task_container>> m_tasks;
    std::vector<std::thread> m_threads;
    std::mutex m_mutex;
};
}
