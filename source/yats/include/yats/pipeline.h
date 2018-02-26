#pragma once

#include <memory>
#include <string>
#include <vector>

#include <yats/lambda_task.h>
#include <yats/task_configurator.h>
#include <yats/util.h>

namespace yats
{

class pipeline
{
public:
    pipeline() = default;
    pipeline(const pipeline& other) = delete;
    pipeline(pipeline&& other) = default;

    ~pipeline() = default;

    pipeline& operator=(const pipeline& other) = delete;
    pipeline& operator=(pipeline&& other) = default;

    template <typename LambdaTask>
    auto* add(LambdaTask task)
    {
        using type = decltype(make_lambda_task(&LambdaTask::operator()));
        return add<type>(typename type::function_type(task));
    }

    template <typename Task, typename... Parameters>
    task_configurator<Task, Parameters...>* add(Parameters&&... parameters)
    {
        static_assert(has_unique_ids_v<typename decltype(make_helper(&Task::run))::input_tuple>, "Can not add Task because multiple Inputs share the same Id.");
        static_assert(has_unique_ids_v<typename decltype(make_helper(&Task::run))::output_tuple>, "Can not add Task because multiple Outputs share the same Id.");

        m_tasks.push_back(std::make_unique<task_configurator<Task, Parameters...>>(std::forward<Parameters>(parameters)...));
        return static_cast<task_configurator<Task, Parameters...>*>(m_tasks.back().get());
    }

    std::vector<std::unique_ptr<abstract_task_container>> build() const
    {
        auto constraint_map = convert_constraints();

        std::vector<std::unique_ptr<abstract_connection_helper>> helpers;
        for (const auto& configurator : m_tasks)
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

            std::vector<size_t> constraints;
            for (const auto& constraint_name : m_tasks[i]->thread_constraints().names())
            {
                constraints.push_back(constraint_map.at(constraint_name));
            }
            tasks.back()->set_constraints(constraints);
        }

        return tasks;
    }

    size_t constraint_count() const
    {
        //TODO kinda inefficient
        return convert_constraints().size();
    }

protected:
    std::map<std::string, size_t> convert_constraints() const
    {
        std::map<std::string, size_t> constraint_map{
            { thread_group::any_thread_name(), thread_group::any_thread_number() },
            { thread_group::main_thread_name(), thread_group::main_thread_number() }
        };

        size_t next_constraint = 2;
        for (const auto& task : m_tasks)
        {
            for (const auto& constraint_name : task->thread_constraints().names())
            {
                auto success = constraint_map.emplace(constraint_name, next_constraint);
                if (success.second)
                {
                    ++next_constraint;
                }
            }
        }

        return constraint_map;
    }

    std::vector<std::unique_ptr<abstract_task_configurator>> m_tasks;
};
}
