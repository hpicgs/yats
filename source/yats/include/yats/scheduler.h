#pragma once

#include <cassert>
#include <mutex>
#include <vector>

#include <yats/pipeline.h>
#include <yats/task_container.h>
#include <yats/thread_pool.h>

namespace yats
{
class scheduler
{
public:
    explicit scheduler(const pipeline& pipeline, size_t number_of_threads = 4)
        : m_condition(number_of_threads)
        , m_thread_pool(m_condition)
        , m_tasks(pipeline.build())
    {

        static const std::string unnamed_thread_pool;
        for (size_t i = 0; i < number_of_threads; ++i)
        {
            m_thread_pool.execute([this]() mutable
            {
                auto current_task = get(unnamed_thread_pool);
                m_tasks[current_task]->run();

                // This can be removed after the change of control flow.
                schedule(current_task);
            }, unnamed_thread_pool);
        }
    }

    scheduler(const scheduler& other) = delete;
    scheduler(scheduler&& other) = delete;

    ~scheduler() = default;

    scheduler& operator=(const scheduler& other) = delete;
    scheduler& operator=(scheduler&& other) = delete;

    void run()
    {
        initial_schedule();

        static const std::string main_thread_pool = "main";
        while (!m_condition.has_finished())
        {
            if (auto guard = m_condition.wait(main_thread_pool))
            {
                auto current_task = get(main_thread_pool);
                m_tasks[current_task]->run();

                // This can be removed after the change of control flow.
                schedule(current_task);
            }
            else
            {
                break;
            }
        }
    }

protected:
    size_t get(const std::string& constraint)
    {
        std::unique_lock<std::mutex> guard(m_mutex);
        auto current_task = m_tasks_to_process[constraint].front();
        m_tasks_to_process[constraint].pop();
        return current_task;
    }

    void schedule(size_t index)
    {
        std::unique_lock<std::mutex> guard(m_mutex);
        for (auto next_task : m_tasks[index]->following_nodes())
        {
            if (m_tasks[next_task]->can_run())
            {
                auto& constraint = m_tasks[next_task]->constraint().thread_identifier;
                m_tasks_to_process[constraint].push(next_task);
                m_condition.notify(constraint);
            }
        }
    }

    void initial_schedule()
    {
        for (size_t i = 0; i < m_tasks.size(); ++i)
        {
            if (m_tasks[i]->can_run())
            {
                auto& constraint = m_tasks[i]->constraint().thread_identifier;
                m_tasks_to_process[constraint].push(i);
                m_condition.notify(constraint);
            }
        }
    }

    condition m_condition;
    thread_pool m_thread_pool;

    std::vector<std::unique_ptr<abstract_task_container>> m_tasks;
    std::map<std::string, std::queue<size_t>> m_tasks_to_process;

    std::mutex m_mutex;
};
}
