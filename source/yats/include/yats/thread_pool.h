#pragma once

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <queue>

#include <yats/task_container.h>

namespace yats
{
class abstract_thread_pool_observer
{
public:
    abstract_thread_pool_observer() = default;
    abstract_thread_pool_observer(const abstract_thread_pool_observer& other) = delete;
    abstract_thread_pool_observer(abstract_thread_pool_observer&& other) = delete;

    abstract_thread_pool_observer& operator=(const abstract_thread_pool_observer& other) = delete;
    abstract_thread_pool_observer& operator=(abstract_thread_pool_observer&& other) = delete;

    virtual ~abstract_thread_pool_observer() = default;
    
    virtual void task_executed(abstract_task_container* task) = 0;
};

class thread_pool
{
public:
    explicit thread_pool(size_t thread_count)
        : m_is_cancellation_requested(false), m_observer(nullptr)
    {
        m_threads.reserve(thread_count);
        for (size_t i = 0; i < thread_count; i++)
        {
            m_threads.emplace_back(&thread_pool::thread_function, this);
        }
    }

    thread_pool(const thread_pool& other) = delete;
    thread_pool(thread_pool&& other) = delete;

    thread_pool& operator=(const thread_pool& other) = delete;
    thread_pool& operator=(thread_pool&& other) = delete;

    ~thread_pool()
    {
        terminate();
    }

    /**
     * Executes task in an own thread as soon as a
     * thread of the pool is available.
     * @param task Pointer to task to execute
     */
    void execute(abstract_task_container * task)
    {
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_task_queue.push(task);
        }
        m_task_added.notify_one();
    }

    /**
     * Terminates the thread pool. The thread pool is terminated
     * as soon as all threads have processed their current task.
     * This functions returns after all threads have terminated.
     */
    void terminate()
    {
        m_is_cancellation_requested = true;
        m_task_added.notify_all();
        join();
    }

    void subscribe(abstract_thread_pool_observer* observer)
    {
        m_observer = observer;
    }

protected:
    std::vector<std::thread> m_threads;
    std::queue <abstract_task_container*> m_task_queue;
    std::atomic_bool m_is_cancellation_requested;
    std::mutex m_mutex;
    std::condition_variable m_task_added;
    abstract_thread_pool_observer* m_observer;

    /**
     * Joins all threads of the thread pool.
     */
    void join()
    {
        for (auto & thread : m_threads)
        {
            thread.join();
        }

        m_threads.clear();
    }

    /**
     * Notifies observer that a task was executed
     * @param task Task, which was executed.
     */
    void on_task_executed(abstract_task_container* task) const
    {
        if (m_observer != nullptr)
        {
            m_observer->task_executed(task);
        }
    }

    void thread_function()
    {
        abstract_task_container* task;
        while(!m_is_cancellation_requested)
        {
            {
                std::unique_lock<std::mutex> lock(m_mutex);
                m_task_added.wait(lock, [this] {
                    return !m_task_queue.empty() || m_is_cancellation_requested;
                });
                if (m_is_cancellation_requested)
                {
                    break;
                }

                task = m_task_queue.front();
                m_task_queue.pop();
            }

            task->run();
            on_task_executed(task);
        }
    }
};
}
