#pragma once

#include <cassert>
#include <mutex>
#include <vector>

#include <yats/pipeline.h>
#include <yats/task_container.h>
#include <yats/thread_pool.h>

namespace yats
{
class scheduler : public abstract_thread_pool_observer
{
public:
    explicit scheduler(const pipeline& pipeline, size_t number_of_threads = 4)
        : m_tasks(pipeline.build()), m_thread_pool(number_of_threads)
    {      
        m_thread_pool.subscribe(this);
        
        // Determines all tasks wich can be run right from the start.
        for (auto & task : m_tasks)
        {
            if (task->can_run())
            {
                m_initial_tasks.push_back(task.get());
            }
        }
    }

    scheduler(const scheduler& other) = delete;
    scheduler(scheduler&& other) = delete;

    ~scheduler() = default;

    scheduler& operator=(const scheduler& other) = delete;
    scheduler& operator=(scheduler&& other) = delete;

    void run()
    {
        size_t tasks_processed = 0;
        m_scheduled_task_count = 0;
     
        if (!m_tasks.empty() && m_initial_tasks.empty())
        {
            throw std::runtime_error("No schedulable initial task found.");
        }

        schedule_initial_tasks();

        while (tasks_processed < m_tasks.size())
        {
            if (m_scheduled_task_count == 0)
            {
                throw std::runtime_error("No schedulable task found.");
            }

            abstract_task_container* task;
            {
                std::unique_lock<std::mutex> lock(m_mutex);
                m_wait_for_task_executed.wait(lock, [this] {return !m_tasks_to_process.empty(); });

                task = m_tasks_to_process.front();
                m_tasks_to_process.pop();
            }

            schedule_following_tasks(task);
            ++tasks_processed;
            --m_scheduled_task_count;
        }
        assert(m_tasks_to_process.empty());
    }

    /**
    * Called by the thread pool after task has been executed.
    * @param task Task which was executed.
    */
    void task_executed(abstract_task_container* task) override
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_tasks_to_process.push(task);
        m_wait_for_task_executed.notify_one();
    }

protected:
    // Stores all task_containers with their position as an implicit id
    std::vector<std::unique_ptr<abstract_task_container>> m_tasks;
    std::vector<abstract_task_container*> m_initial_tasks;
    yats::thread_pool m_thread_pool;
    std::condition_variable m_wait_for_task_executed;
    std::queue<abstract_task_container*> m_tasks_to_process;
    std::mutex m_mutex;
    std::size_t m_scheduled_task_count;

    /**
     * Schedules all initial tasks (tasks, that are runnable from the start),
     * to be executed as soon as a thread is available.
     */
    void schedule_initial_tasks()
    {
        for (auto & task : m_initial_tasks)
        {
            schedule(task);
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
    void schedule_if_runnable(size_t index)
    {
        if (m_tasks[index]->can_run())
        {
            schedule(m_tasks[index].get());
        }
    }

    void schedule(abstract_task_container* task)
    {
        ++m_scheduled_task_count;
        m_thread_pool.execute(task);
    }
};
}
