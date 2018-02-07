#pragma once

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <queue>

#include <yats/task_container.h>

// Task container execution pool.
namespace yats
{
class thread_pool
{
public:
    /**
     * Constructs a new thread pool.
     * @param thread_count Number of threads to use
     */
    explicit thread_pool(const size_t thread_count)
        : m_is_cancellation_requested(false), m_is_shutdown_requested(false)
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
     * Executes function_to_execute in an own thread as soon as a
     * thread of the pool is available.
     * @param function_to_execute void() function that is to be executed in
     * a thread
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

    /**
     * Waits for the thread pool to process all tasks and terminates
     * the threads afterwards.
     * This function returns as all threads have been terminated.
     */
    void wait()
    {
        m_is_shutdown_requested = true;
        m_task_added.notify_all();
        join();
    }

    void subscribe(const std::function<void(abstract_task_container*)> & listener)
    {
        m_listeners.push_back(listener);
    }

protected:
    std::vector<std::thread> m_threads;
    std::queue <abstract_task_container*> m_task_queue;
    std::atomic_bool m_is_cancellation_requested;
    std::atomic_bool m_is_shutdown_requested;
    std::mutex m_mutex;
    std::condition_variable m_task_added;
    std::vector<std::function<void(abstract_task_container*)>> m_listeners;

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

    void on_task_executed(abstract_task_container* task)
    {
        for (auto & listener : m_listeners) {
            listener(task);
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
                    return !m_task_queue.empty() || m_is_cancellation_requested || m_is_shutdown_requested;
                });
                if (m_is_cancellation_requested)
                {
                    break;
                }
                if (m_task_queue.empty() && m_is_shutdown_requested)
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
