#pragma once

#include <memory>
#include <string>
#include <vector>

#include <yats/scheduler.h>
#include <yats/task_configurator.h>
#include <yats/util.h>

namespace yats
{

class pipeline
{
public:
    pipeline() = default;

    template <typename Task, typename... Parameters>
    task_configurator<Task, Parameters...>* add(Parameters&&... parameters)
    {
        static_assert(has_unique_ids_v<typename decltype(make_helper(&Task::run))::input_tuple>, "Can not add Task because multiple Inputs share the same Id.");
        static_assert(has_unique_ids_v<typename decltype(make_helper(&Task::run))::output_tuple>, "Can not add Task because multiple Outputs share the same Id.");

        m_tasks.push_back(std::make_unique<task_configurator<Task, Parameters...>>(std::forward<Parameters>(parameters)...));
        return static_cast<task_configurator<Task, Parameters...>*>(m_tasks.back().get());
    }

    void run()
    {
        auto tasks = abstract_task_configurator::build(m_tasks);
    }

    scheduler build()
    {
        return scheduler{ m_tasks };
    }

protected:
    std::vector<std::unique_ptr<abstract_task_configurator>> m_tasks;
};
}
