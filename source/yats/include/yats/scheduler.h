#pragma once

#include <algorithm>
#include <mutex>
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
        : m_tasks(pipeline.build()), m_thread_pool(number_of_threads), m_tasks_processed(0)
    {
        const std::function<void(abstract_task_container*)> callback = [this](abstract_task_container* task)
        {
            task_executed(task);
        };
       
        m_thread_pool.subscribe(callback);
    }

    scheduler(const scheduler& other) = delete;
    scheduler(scheduler&& other) = delete;

    ~scheduler() = default;

    scheduler& operator=(const scheduler& other) = delete;
    scheduler& operator=(scheduler&& other) = delete;

    void run()
    {
        to_run.clear();
        m_tasks_processed = 0;

        for (size_t i = 0; i < m_tasks.size(); i++)
        {
            to_run.push_back(true);
        }

        schedule_all_runnable_tasks();

        while (m_tasks_processed < m_tasks.size())
        {
            abstract_task_container* task;
            {
                std::unique_lock<std::mutex> lock(m_mutex);
                m_wait_for_task_executed.wait(lock, [this] {return !m_tasks_to_process.empty(); });

                task = m_tasks_to_process.front();
                m_tasks_to_process.pop();
                m_tasks_processed++;
            }

            schedule_following_tasks(task);
        }
        ASSERT_EQ(m_tasks_to_process.empty(), true);
    }



protected:
    // Stores all task_containers with their position as an implicit id
    std::vector<std::unique_ptr<abstract_task_container>> m_tasks;
    yats::thread_pool m_thread_pool;
    std::vector<bool> to_run;
    size_t m_tasks_processed;
    std::condition_variable m_wait_for_task_executed;
    std::queue<abstract_task_container*> m_tasks_to_process;
    std::mutex m_mutex;

    /**
     * Schedules any runnable task to be executed as soon
     * a thread is available.
     */
    void schedule_all_runnable_tasks()
    {
        for (size_t i = 0; i < to_run.size(); i++)
        {
            schedule_if_runnable(i);
        }
    }

    /**
     * Schedules all following tasks of task to be executed,
     * if their preconditions are met.
     * @param task Task whose following tasks should be scheduled.
     */
    void schedule_following_tasks(abstract_task_container* task)
    {
        for (auto & i : task->following_nodes())
        {
            schedule_if_runnable(i);
        }
    }

    /**
     * Schedukes the task m_tasks[index] if it may be executed
     * @param index index of task to schedule
     */
    void schedule_if_runnable(const size_t index)
    {
        if (to_run[index] && m_tasks[index]->can_run())
        {
            to_run[index] = false;
            m_thread_pool.execute(m_tasks[index].get());
        }
    }

    /**
     * Called by the thread pool after task has been executed.
     * @param task Task which was executed.
     */
    void task_executed(abstract_task_container* task)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_tasks_to_process.push(task);
        m_wait_for_task_executed.notify_one();
    }

    int index_of(abstract_task_container* task)
    {
        for (size_t i = 0; i  < m_tasks.size(); i++)
        {
            if (m_tasks[i].get()  == task)
            {
                return static_cast<int>(i);
            }
        }
        return -1;
    }
};
}
