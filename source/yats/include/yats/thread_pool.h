#pragma once

#include <atomic>
#include <iostream>
#include <mutex>
#include <queue>

#include <yats/thread_safe_queue.h>

namespace yats
{

class thread_pool
{
public:
    explicit thread_pool(const size_t thread_count)
    {
        std::cout << "initializing" << std::endl;
        m_terminate = false;
        m_threads.reserve(thread_count);
        for (size_t i = 0; i < thread_count; i++)
        {
            m_threads.emplace_back(&thread_pool::foo, this);
        }
        std::cout << "initialized" << std::endl;
    }

    ~thread_pool()
    {
        terminate();
    }

    void execute(const std::function<void()> & function_to_execute)
    {
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_queue.push(function_to_execute);
        }
        m_function_added.notify_one();
    }

    void terminate()
    {
        m_terminate = true;
    }
protected:
    std::vector<std::thread> m_threads;
    std::queue < std::function<void()>> m_queue;
    std::atomic_bool m_terminate;
    std::mutex m_mutex;
    std::condition_variable m_function_added;

    void foo()
    {
        auto thread_id = std::this_thread::get_id();
        std::function<void()> run;
        std::unique_lock<std::mutex> lock(m_mutex, std::defer_lock);
        while(!m_terminate)
        {
            lock.lock();
            if (m_queue.empty())
            {
                std::cout << thread_id << ": " << "waiting" << std::endl;
                m_function_added.wait(lock);
            }
            
            run = m_queue.front();
            m_queue.pop();
            std::cout << thread_id << ": " << "executing" << std::endl;
            lock.unlock();
            run();
        }
    }

};
}
