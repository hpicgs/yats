#pragma once

#include <algorithm>
#include <atomic>
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
        : m_is_running(false)
        , m_externals_finished(false)
        , m_tasks(std::move(pipeline).build([this](abstract_task_container* task, size_t index, bool finished) { task_received_input(task, index, finished); }))
        , m_tasks_to_process(number_of_constraints(m_tasks))
        , m_condition(number_of_threads, number_of_constraints(m_tasks), m_externals_finished)
        , m_thread_pool(m_condition)
    {
        if (number_of_threads == 0)
        {
            throw std::runtime_error("Cannot run scheduler on 0 concurrent tasks!");
        }

        auto run_task_lambda = [this](size_t constraint) mutable
        {
            const auto current_task = get(constraint);

            // Don't continue if this task failed to complete
            if (!run_task(current_task))
            {
                return;
            }

            schedule_following(current_task);
        };

        // We need number_of_threads threads for the any constraint
        for (size_t i = 0; i < number_of_threads; ++i)
        {
            m_thread_pool.execute([run_task_lambda]() mutable { run_task_lambda(thread_group::ANY); }, thread_group::ANY);
        }

        // We need a thread for each user defined constraint
        const auto constraints_count = number_of_constraints(m_tasks);
        for (size_t constraint = thread_group::COUNT; constraint < constraints_count; ++constraint)
        {
            m_thread_pool.execute([run_task_lambda, constraint]() mutable { run_task_lambda(constraint); }, constraint);
        }

        for (const auto& task : m_tasks)
        {
            m_initial_external_status.push_back(task->receives_external_input());
            m_initial_external_status.back().flip();
        }
        m_externals_finished = externals_finished(m_initial_external_status);
    }

    scheduler(const scheduler& other) = delete;
    scheduler(scheduler&& other) = delete;

    ~scheduler() = default;

    scheduler& operator=(const scheduler& other) = delete;
    scheduler& operator=(scheduler&& other) = delete;

    void run()
    {
        m_is_running = true;
        m_current_external_status = m_initial_external_status;
        initial_schedule();

        while (auto guard = m_condition.wait_main())
        {
            const auto current_task = get(thread_group::MAIN);
            run_task(current_task);
            assert_no_task_failed();
            schedule_following(current_task);
        }

        assert_no_task_failed();
        m_is_running = false;
    }

    bool is_running() const
    {
        return m_is_running;
    }

protected:
    size_t get(size_t constraint)
    {
        std::unique_lock<std::mutex> guard(m_mutex);
        const auto current_task = m_tasks_to_process[constraint].front();
        m_tasks_to_process[constraint].pop();
        return current_task;
    }

    void schedule(size_t index)
    {
        m_tasks[index]->reserve_run();
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
        for (const auto next_task : m_tasks[index]->following_nodes())
        {
            if (m_tasks[next_task]->can_run())
            {
                schedule(next_task);
            }
        }
    }

    void initial_schedule()
    {
        for (const auto& external_callback : m_external_callbacks)
        {
            external_callback();
        }
        m_external_callbacks.clear();

        std::unique_lock<std::mutex> guard(m_mutex);
        for (size_t index = 0; index < m_tasks.size(); ++index)
        {
            if (m_tasks[index]->can_run())
            {
                schedule(index);
            }
        }
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
        if (m_task_error)
        {
            try
            {
                m_condition.terminate();
                std::rethrow_exception(m_task_error);
            } catch (const std::exception& exception)
            {
                m_is_running = false;
                throw std::runtime_error(std::string("Error in task:\n\t") + exception.what());
            }
        }
    }
  
    void task_received_input(abstract_task_container* task, size_t index, bool finished)
    {
        if (!m_is_running)
        {
            m_external_callbacks.push_back([this, task, index, finished]()
            {
                task_received_input(task, index, finished);
            });
            return;
        }

        const auto index_of_task_it = std::find_if(m_tasks.cbegin(), m_tasks.cend(), [to_find = task](const std::unique_ptr<abstract_task_container>& task)
        {
            return to_find == task.get();
        });
        const size_t index_of_task = std::distance(m_tasks.cbegin(), index_of_task_it);
        
        std::unique_lock<std::mutex> guard(m_mutex);
        if (m_current_external_status[index_of_task][index])
        {
            return;
        }

        if (finished)
        {
            m_current_external_status[index_of_task][index] = finished;
        }
        m_externals_finished = externals_finished(m_current_external_status);

        // TODO: If the pipeline finished and is only waiting for this one external input
        // and can_run still returns false, then the pipeline will (or at least should) hang forever
        if (m_tasks[index_of_task]->can_run())
        {
            schedule(index_of_task);
        }
    }

    static bool externals_finished(const std::vector<std::vector<bool>>& externals)
    {
        return std::all_of(externals.cbegin(), externals.cend(), [](const std::vector<bool>& inner)
        {
            return std::all_of(inner.cbegin(), inner.cend(), [](bool finished) { return finished; });
        });
    }

    static size_t number_of_constraints(const std::vector<std::unique_ptr<abstract_task_container>>& tasks)
    {
        size_t max_constraint = 0;
        for (const auto& task : tasks)
        {
            max_constraint = std::max(max_constraint, *std::max_element(task->constraints().cbegin(), task->constraints().cend()));
        }

        // We have at least thread_group::COUNT constraints
        // even if no task uses one of the predefined constraints
        // If a task uses a user defined constraint we have to add 1 because constraints start at 0
        return std::max<size_t>(thread_group::COUNT, max_constraint + 1);
    }

    std::atomic_bool m_is_running;
    std::atomic_bool m_externals_finished;

    std::mutex m_mutex;

    std::vector<std::unique_ptr<abstract_task_container>> m_tasks;
    std::vector<std::queue<size_t>> m_tasks_to_process;

    condition m_condition;
    thread_pool m_thread_pool;

    std::exception_ptr m_task_error;

    std::vector<std::vector<bool>> m_initial_external_status;
    std::vector<std::vector<bool>> m_current_external_status;
    std::vector<std::function<void()>> m_external_callbacks;
};
}
