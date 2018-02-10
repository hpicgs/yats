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
        : m_tasks(pipeline.build()), m_thread_pool(number_of_threads), m_may_run(false),
          m_terminate(false), m_scheduled_task_count(0)
    {      
        m_thread_pool.subscribe(this);
        m_task_distribution_thread = std::thread(&scheduler::distribute_tasks, this);
        
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

    ~scheduler()
    {
        {
            std::lock_guard<std::mutex> lock(m_mutex_run);
            m_terminate = true;
            m_wait_for_run.notify_one();
        }
        m_task_distribution_thread.join();
    }

    scheduler& operator=(const scheduler& other) = delete;
    scheduler& operator=(scheduler&& other) = delete;

    void run()
    {
        m_tasks_processed = 0;
        m_scheduled_task_count = 0;
     
        if (!m_tasks.empty() && m_initial_tasks.empty())
        {
            throw std::runtime_error("No schedulable initial task found.");
        }

        m_may_run = true;
        {
            std::lock_guard<std::mutex> lock(m_mutex_run);
            m_wait_for_run.notify_one();
        }

        while(m_tasks_processed < m_tasks.size())
        {
            abstract_task_container* task = nullptr;
            {
                std::unique_lock<std::mutex> lock(m_mutex_main_thread);
                m_wait_for_main_thread_task.wait(lock, [this]()
                {
                    return m_tasks_processed <= m_tasks.size();
                });

                if (!m_tasks_to_process_in_main_thread.empty())
                {
                    task = m_tasks_to_process_in_main_thread.front();
                    m_tasks_to_process_in_main_thread.pop();
                }
            }

            if (task != nullptr)
            {
                task->run();
                task_executed(task);
            }
        }
    }
    
    /**
    * Called by the thread pool after task has been executed.
    * @param task Task which was executed.
    */
    void task_executed(abstract_task_container* task) override
    {
        std::lock_guard<std::mutex> lock(m_mutex_thread_pool);
        m_tasks_to_process.push(task);
        m_wait_for_task_executed.notify_one();
    }

protected:
    // Stores all task_containers with their position as an implicit id
    std::vector<std::unique_ptr<abstract_task_container>> m_tasks;
    std::vector<abstract_task_container*> m_initial_tasks;
    yats::thread_pool m_thread_pool;
    std::mutex m_mutex_thread_pool;
    std::condition_variable m_wait_for_task_executed;
    std::queue<abstract_task_container*> m_tasks_to_process;

    std::condition_variable m_wait_for_main_thread_task;
    std::mutex m_mutex_main_thread;
    std::queue<abstract_task_container*> m_tasks_to_process_in_main_thread;

    std::thread m_task_distribution_thread;
    std::condition_variable m_wait_for_run;
    std::mutex m_mutex_run;
    bool m_may_run;
    bool m_terminate;
    
    std::size_t m_scheduled_task_count;
    std::size_t m_tasks_processed;

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

    void distribute_tasks()
    {
        while (true)
        {
            {
                std::unique_lock<std::mutex> lock(m_mutex_run);
                m_wait_for_run.wait(lock, [this]() {return m_may_run || m_terminate; });
                m_may_run = false;
            }

            if (m_terminate)
            {
                break;
            }

            if (!m_tasks.empty() && m_initial_tasks.empty())
            {
                throw std::runtime_error("No schedulable initial task found.");
            }

            schedule_initial_tasks();

            while (m_tasks_processed < m_tasks.size())
            {
                //Todo: Exception handling.
                if (m_scheduled_task_count == 0)
                {
                    throw std::runtime_error("No schedulable task found.");
                }

                abstract_task_container* task;
                {
                    std::unique_lock<std::mutex> lock(m_mutex_thread_pool);
                    m_wait_for_task_executed.wait(lock, [this] {return !m_tasks_to_process.empty(); });

                    task = m_tasks_to_process.front();
                    m_tasks_to_process.pop();
                }

                schedule_following_tasks(task);
                ++m_tasks_processed;
                --m_scheduled_task_count;

                {
                    std::lock_guard<std::mutex> lock(m_mutex_main_thread);
                    m_wait_for_main_thread_task.notify_one();
                }
            }
            assert(m_tasks_to_process.empty());
        }
    }
};
}
