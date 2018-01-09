#pragma once

#include <memory>
#include <string>
#include <vector>

#include <yats/task_configurator.h>
#include <yats/util.h>

namespace yats
{

class pipeline
{
public:
    pipeline() = default;

    template <typename Task>
    task_configurator<Task>* add()
    {
        static_assert(has_unique_ids_v<typename decltype(make_helper(&Task::run))::input_tuple>, "Can not add Task because multiple Inputs share the same Id.");
        static_assert(has_unique_ids_v<typename decltype(make_helper(&Task::run))::output_tuple>, "Can not add Task because multiple Outputs share the same Id.");

        m_tasks.push_back(std::make_unique<task_configurator<Task>>());
        return static_cast<task_configurator<Task>*>(m_tasks.back().get());
    }

    std::vector<std::unique_ptr<abstract_task_container>> build() const
    {
        std::vector<std::unique_ptr<abstract_connection_helper>> helpers;
        for (auto& configurator : m_tasks)
        {
            helpers.emplace_back(configurator->construct_connection_helper());
        }

        std::map<const abstract_output_connector*, size_t> output_owner;
        for (size_t i = 0; i < m_tasks.size(); ++i)
        {
            auto outputs = helpers[i]->outputs();
            for (auto output : outputs)
            {
                output_owner.emplace(output.first, i);
            }
        }

        for (size_t i = 0; i < helpers.size(); ++i)
        {
            auto inputs = helpers[i]->inputs();
            for (auto input : inputs)
            {
                auto source_location = input.first->output();
                auto source_task_id = output_owner.at(source_location);

                helpers[source_task_id]->bind(source_location, helpers[i]->target(input.first));
                helpers[source_task_id]->add_following(i);
            }
        }

        std::vector<std::unique_ptr<abstract_task_container>> tasks;
        for (size_t i = 0; i < m_tasks.size(); ++i)
        {
            tasks.push_back(m_tasks[i]->construct_task_container(std::move(helpers[i])));
        }

        return tasks;
    }

protected:
    std::vector<std::unique_ptr<abstract_task_configurator>> m_tasks;
};
}
