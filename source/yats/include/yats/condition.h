#pragma once

#include <condition_variable>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

#include <yats/constraint.h>

namespace yats
{

/**
 * Helper class to control execution and waiting of tread_pools. Wraps a std::condition_variable.
 */
class condition
{
public:
    condition(size_t number_of_threads, size_t number_of_constraints, const std::atomic_bool& externals_finished)
        : m_number_of_threads(number_of_threads)
        , m_thread_identifier(number_of_constraints)
        , m_is_active(true)
        , m_task_added(number_of_constraints + 1)
        , m_notify_count(number_of_constraints + 1, 0)
        , m_externals_finished(externals_finished)
    {
        m_notify_count[m_thread_identifier] = m_number_of_threads;
    }

    /**
     * RAII wrapper for athread resource. Used to limit the maximum amount of concurrent tasks.
     */
    class thread_guard
    {
    public:
        thread_guard(condition* condition, bool is_active, std::unique_lock<std::mutex>& guard)
            : m_condition(condition)
            , m_is_active(is_active)
        {
            // Wait on the resource condition.
            while (condition->m_notify_count[m_condition->m_thread_identifier] == 0 && m_is_active)
            {
                condition->m_task_added[m_condition->m_thread_identifier].wait(guard);
            }
            --condition->m_notify_count[m_condition->m_thread_identifier];
        }

        thread_guard(const thread_guard&) = delete;
        thread_guard(thread_guard&&) = default;

        ~thread_guard()
        {
            m_condition->notify(m_condition->m_thread_identifier);
            if (m_is_active)
            {
                m_condition->check_finish();
            }
        }

        thread_guard& operator=(const thread_guard&) = delete;
        thread_guard& operator=(thread_guard&&) = default;

        operator bool() const
        {
            return m_is_active;
        }

    protected:
        condition* m_condition;
        const bool m_is_active;
    };

    thread_guard wait(size_t constraint)
    {
        std::unique_lock<std::mutex> guard(m_mutex);

        // Wait on the resource condition.
        while (m_notify_count[constraint] == 0 && m_is_active)
        {
            m_task_added[constraint].wait(guard);
        }
        --m_notify_count[constraint];

        return thread_guard(this, m_is_active, guard);
    }

    thread_guard wait_main()
    {
        std::unique_lock<std::mutex> guard(m_mutex);

        if (has_finished())
        {
            return thread_guard(this, false, guard);
        }

        // Wait on the resource condition.
        while (m_notify_count[thread_group::MAIN] == 0 && m_is_active)
        {
            m_task_added[thread_group::MAIN].wait(guard);
            if (has_finished())
            {
                return thread_guard(this, false, guard);
            }
        }
        --m_notify_count[thread_group::MAIN];

        return thread_guard(this, m_is_active, guard);
    }

    void notify(size_t constraint)
    {
        std::unique_lock<std::mutex> guard(m_mutex);
        ++m_notify_count[constraint];
        m_task_added[constraint].notify_one();
    }

    void check_finish()
    {
        std::unique_lock<std::mutex> guard(m_mutex);
        if (has_finished())
        {
            // Notify but do not increment m_notify_count.
            m_task_added[thread_group::MAIN].notify_one();
        }
    }

    void terminate()
    {
        std::unique_lock<std::mutex> guard(m_mutex);
        m_is_active = false;
        for (auto& elem : m_task_added)
        {
            elem.notify_all();
        }
    }

    bool is_active() const
    {
        return m_is_active;
    }

    bool has_finished()
    {
        auto count = std::accumulate(m_notify_count.cbegin(), m_notify_count.cend(), 0ull);
        return count == m_number_of_threads && m_notify_count[m_thread_identifier] == m_number_of_threads && m_externals_finished;
    }

    const size_t m_number_of_threads;
    const size_t m_thread_identifier;

    bool m_is_active;
    std::vector<std::condition_variable> m_task_added;
    std::vector<size_t> m_notify_count;
    std::mutex m_mutex;

    const std::atomic_bool& m_externals_finished;
};
}
