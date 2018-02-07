#pragma once

#include <algorithm>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include <yats/pipeline.h>
#include <yats/thread_pool.h>
#include <yats/task_container.h>

namespace yats
{

class scheduler
{
public:
    explicit scheduler(const pipeline& pipeline, const size_t number_of_threads = 4)
        : m_tasks(pipeline.build()), m_thread_pool(number_of_threads)
    {
        const std::function<void(abstract_task_container*)> f = [this](abstract_task_container* bla)
        {
            task_executed(bla);
        };
       
        m_thread_pool.subscribe(f);
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

        while (!to_run.empty())
        {
            abstract_task_container* task;
            {
                std::unique_lock<std::mutex> lock(m_mutex);
                m_wait_for_task_executed.wait(lock, [this] {return !m_tasks_to_process.empty(); });

                task = m_tasks_to_process.front();
                m_tasks_to_process.pop();
            }


            to_run.erase(std::remove(to_run.begin(), to_run.end(), task));

            check_runnable(task);
        }
        ASSERT_EQ(m_tasks_to_process.empty(), true);
    }



protected:
    // Stores all task_containers with their position as an implicit id
    std::vector<std::unique_ptr<abstract_task_container>> m_tasks;
    yats::thread_pool m_thread_pool;
    std::vector<abstract_task_container*> to_run;
    std::condition_variable m_wait_for_task_executed;
    std::queue<abstract_task_container*> m_tasks_to_process;
    std::mutex m_mutex;

    void check_runnable()
    {
        for (auto & task : to_run) {
            if (task->can_run())
            {
                m_thread_pool.execute(task);
            }
        }
    }

    void check_runnable(abstract_task_container* task)
    {
        // Add task.following_nodes() to to_run.
        check_runnable();
    }

    void task_executed(abstract_task_container* task)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_tasks_to_process.push(task);
        m_wait_for_task_executed.notify_one();
    }

};
}
