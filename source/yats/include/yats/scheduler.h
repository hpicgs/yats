#pragma once

#include <algorithm>
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
        : m_tasks(std::move(pipeline).build([this](abstract_task_container* task) { task_received_input(task); }))
        , m_tasks_to_process(number_of_constraints(m_tasks))
        , m_condition(number_of_threads, number_of_constraints(m_tasks))
        , m_thread_pool(m_condition)
    {
        for (size_t i = 0; i < number_of_threads; ++i)
        {
            m_thread_pool.execute([this]() mutable {
                auto current_task = get(thread_group::ANY);

                // Don't continue if this task failed to complete
                if (!run_task(current_task)) {
                    return;
                }

                schedule_following(current_task);
            },
            thread_group::ANY);
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
            run_task(current_task);
            assert_no_task_failed();
            schedule_following(current_task);
        }

        assert_no_task_failed();
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
        // TODO: there's probably a better method to check which thread to take
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
        std::unique_lock<std::mutex> guard(m_mutex);
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
        return std::max<size_t>(2, max_constraint);
    }

    bool run_task(const size_t task_id)
    {
        auto& task = m_tasks[task_id];

        task->run();

        if (task->failed())
        {
            m_task_error = task->get_error();
            return false;
        }

        return true;
    }

    void assert_no_task_failed()
    {
        // If a task failed, we want to catch this in the main thread and throw an error for the user.
        if (m_task_error) {
            try
            {
                m_condition.terminate();
                std::rethrow_exception(m_task_error);
            } catch (const std::exception& exception)
            {
                throw std::runtime_error(std::string("Error in task:\n\t") + exception.what());
            }
        }
    }
  
    void task_received_input(abstract_task_container* task)
    {
        (void)task;
        // TODO: the scheduler should check if this can run and schedule it
        // it is not implemented yet, because the current scheduler can not do this easily
        // and we change the scheduler right now anyway
    }

    std::vector<std::unique_ptr<abstract_task_container>> m_tasks;
    std::vector<std::queue<size_t>> m_tasks_to_process;

    condition m_condition;
    thread_pool m_thread_pool;

    std::mutex m_mutex;

    std::exception_ptr m_task_error;
};
}
