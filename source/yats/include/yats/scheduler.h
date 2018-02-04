#pragma once

#include <algorithm>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include <yats/pipeline.h>
#include <yats/task_configurator.h>
#include <yats/thread_pool.h>

namespace yats
{

class scheduler
{
public:
    explicit scheduler(const pipeline& pipeline)
        : scheduler(pipeline, 4)
    {
    }

    explicit scheduler(const pipeline& pipeline, const size_t number_of_threads)
        : m_tasks(pipeline.build()), m_thread_pool(number_of_threads)
    {
    }

    scheduler(const scheduler& other) = delete;
    scheduler(scheduler&& other) = delete;

    ~scheduler() = default;

    scheduler& operator=(const scheduler& other) = delete;
    scheduler& operator=(scheduler&& other) = delete;

    void run()
    {
        
        for (auto& elem : m_tasks)
        {
            to_run.push_back(elem.get());
        }

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

        m_thread_pool.wait();
    }

protected:
    // Stores all task_containers with their position as an implicit id
    std::vector<std::unique_ptr<abstract_task_container>> m_tasks;
    yats::thread_pool m_thread_pool;
    std::vector<abstract_task_container*> to_run;
    std::mutex m_mutex;

    void check_runnable()
    {
        while (true)
        {
            const auto runnable = std::find_if(to_run.begin(), to_run.end(), [](abstract_task_container* task) {
                return task->can_run();
            });

            if (runnable == to_run.end())
            {
                return;
            }
            auto task = *runnable;
            to_run.erase(runnable);
            m_thread_pool.execute([task] {task->run(); }, [this](void* v) {task_executed(v); }, task);
        }
    };

    void task_executed(void * object)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto task = static_cast<abstract_task_container*>(object);
        std::cout << "task executed!" << std::endl;
        check_runnable();
    }
};
}