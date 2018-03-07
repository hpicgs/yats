#pragma once

#include <memory>
#include <fstream>
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

    /**
     * Builds the pipeline
     * <p>
     * Note: Following tasks are referenced by the position of the task in the returned vector
     * </p>
     */
    std::vector<std::unique_ptr<abstract_task_container>> build() const
    {
        // required to gain access to the input and output connectors
        std::vector<std::unique_ptr<abstract_connection_helper>> helpers;
        for (const auto& configurator : m_tasks)
        {
            helpers.emplace_back(configurator->construct_connection_helper());
        }

        // Map output to helper index
        // outputs are unique
        std::map<const abstract_output_connector*, size_t> output_owner;
        for (size_t i = 0; i < m_tasks.size(); ++i)
        {
            auto outputs = helpers[i]->outputs();
            for (const auto output : outputs)
            {
                output_owner.emplace(output.first, i);
            }
        }

        for (size_t i = 0; i < helpers.size(); ++i)
        {
            auto inputs = helpers[i]->inputs();
            for (const auto input : inputs)
            {
                auto source_location = input.first->output();
                const auto source_task_id = output_owner.at(source_location);

                // connect output to input
                helpers[source_task_id]->bind(source_location, helpers[i]->target(input.first));
                // tasks connected to an output are successors 
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

    void save_to_file(const std::string& filename)
    {
        // hier werden die helper angelegt
        std::vector<std::unique_ptr<abstract_connection_helper>> helpers;
        for (const auto& configurator : m_tasks)
        {
            helpers.emplace_back(configurator->construct_connection_helper());
        }

        // gehe durch alle Tasks
        // gehe durch alle inputs und outputs -> stehen in den connection_helpern.

        // pro connection helper können wir schon einmal ein entsprechendes struct anlegen.
        // NODE_NAME [label = "NODE_NAME|{{<KEY1>INPUT1|<KEY2>INPUT2...}|{<KEY3>OUTPUT1|<KEY4>OUTPUT2...}}"];
        const std::string node_line_mask = "%s [label = \"%s|{{%s}|{%s}}\"]";
        // get_input_id (index 1, 2, 3, ...)

        std::ofstream file;
        file.open(filename, std::ios_base::out | std::ios_base::trunc);
        std::string input_ids;
        std::string output_ids;
        
 
        for (size_t i = 0; i < helpers.size(); ++i)
        {
           // helpers[i].in
        }

        file.close();

        // Als nächstes müssen die Verbindungen gesetzt werden:
        // ein zweiter loop über alle helper um die Verbindung anzulegen.
        // Wir haben inputs, die nicht von einem output geschrieben werden
        // Outputs, die keinen Input schreiben
        // und outputs, die in Inputs schreiben (normal).

    }

    static std::string inputs_to_string(abstract_connection_helper& helper)
    {
        for (const auto& input : helper.inputs())
        {
            
        }
    }

protected:
    std::vector<std::unique_ptr<abstract_task_configurator>> m_tasks;
};
}
