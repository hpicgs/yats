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
    explicit scheduler(pipeline pipeline, size_t number_of_threads = std::max(std::thread::hardware_concurrency(), 1u))
        : m_tasks(std::move(pipeline).build())
        , m_tasks_to_process(number_of_constraints(m_tasks))
        , m_condition(number_of_threads, number_of_constraints(m_tasks))
        , m_thread_pool(m_condition)
    {
        for (size_t i = 0; i < number_of_threads; ++i)
        {
            m_thread_pool.execute([this]() mutable {
                auto current_task = get(thread_group::ANY);
                m_tasks[current_task]->run();

                schedule_following(current_task);
            }, thread_group::ANY);
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

        while (auto guard = m_condition.wait_main(thread_group::MAIN))
        {
            auto current_task = get(thread_group::MAIN);
            m_tasks[current_task]->run();

            schedule_following(current_task);
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
        const auto& constraints = m_tasks[index]->constraints();

        // If there are multiple threads we choose the one with the smallest current workload
        // TODO: theres probably a better method to check which thread to take
        auto constraint_it = std::min_element(constraints.cbegin(), constraints.cend(), [this](size_t lhs, size_t rhs) {
            return m_tasks_to_process[lhs].size() < m_tasks_to_process[rhs].size();
        });

        m_tasks_to_process[*constraint_it].push(index);
        m_condition.notify(*constraint_it);
    }

    void schedule_following(size_t index)
    {
        std::unique_lock<std::mutex> guard(m_mutex);
        for (auto next_task : m_tasks[index]->following_nodes())
        {
            if (m_tasks[next_task]->can_run())
            {
                schedule(next_task);
            }
        }
    }

    void initial_schedule()
    {
        for (size_t index = 0; index < m_tasks.size(); ++index)
        {
            if (m_tasks[index]->can_run())
            {
                schedule(index);
            }
        }
    }

    static size_t number_of_constraints(const std::vector<std::unique_ptr<abstract_task_container>>& tasks)
    {
        size_t max_constraint = 0;
        for (const auto& task : tasks)
        {
            max_constraint = std::max(max_constraint, *std::max_element(task->constraints().cbegin(), task->constraints().cend()));
        }
        // We have to add the number of constraints which exist even though they are not chosen
        return max_constraint + thread_group::COUNT;
    }

    std::vector<std::unique_ptr<abstract_task_container>> m_tasks;
    std::vector<std::queue<size_t>> m_tasks_to_process;

    condition m_condition;
    thread_pool m_thread_pool;

    std::mutex m_mutex;
};
}
