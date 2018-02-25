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
    explicit scheduler(const pipeline& pipeline, size_t number_of_threads = std::thread::hardware_concurrency())
        : m_tasks(pipeline.build())
        , m_tasks_to_process(pipeline.constraint_count())
        , m_condition(number_of_threads, pipeline.constraint_count())
        , m_thread_pool(m_condition)
    {
        for (size_t i = 0; i < number_of_threads; ++i)
        {
            m_thread_pool.execute([this]() mutable {
                auto current_task = get(thread_group::any_thread_number());
                m_tasks[current_task]->run();

                // This can be removed after the change of control flow.
                schedule(current_task);
            }, thread_group::any_thread_number());
        }
    }

    scheduler(const scheduler& other) = delete;
    scheduler(scheduler&& other) = delete;

    ~scheduler() = default;

    scheduler& operator=(const scheduler& other) = delete;
    scheduler& operator=(scheduler&& other) = delete;

    void run()
    {
        if (!initial_schedule())
        {
            return;
        }

        while (auto guard = m_condition.wait_main(thread_group::main_thread_number()))
        {
            auto current_task = get(thread_group::main_thread_number());
            m_tasks[current_task]->run();

            // This can be removed after the change of control flow.
            schedule(current_task);
        }
    }

protected:
    size_t get(size_t constraint)
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
                const auto& constraints = m_tasks[next_task]->constraints();

                // If there are multiple threads we can chose from take the one with the smallest current workload
                auto constraint_it = std::min_element(constraints.cbegin(), constraints.cend(), [this](size_t lhs, size_t rhs) {
                    return m_tasks_to_process[lhs] < m_tasks_to_process[rhs];
                });

                m_tasks_to_process[*constraint_it].push(next_task);
                m_condition.notify(*constraint_it);
            }
        }
    }

    bool initial_schedule()
    {
        bool active = false;
        for (size_t i = 0; i < m_tasks.size(); ++i)
        {
            if (m_tasks[i]->can_run())
            {
                const auto& constraints = m_tasks[i]->constraints();

                // If there are multiple threads we can chose from take the one with the smallest current workload
                // TODO: theres probably a better method to check which thread to take
                auto constraint_it = std::min_element(constraints.cbegin(), constraints.cend(), [this](size_t lhs, size_t rhs) {
                    return m_tasks_to_process[lhs] < m_tasks_to_process[rhs];
                });

                m_tasks_to_process[*constraint_it].push(i);
                m_condition.notify(*constraint_it);
                active = true;
            }
        }

        return active;
    }

    std::vector<std::unique_ptr<abstract_task_container>> m_tasks;
    std::vector<std::queue<size_t>> m_tasks_to_process;

    condition m_condition;
    thread_pool m_thread_pool;

    std::mutex m_mutex;
};
}
