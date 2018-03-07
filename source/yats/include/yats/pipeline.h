#pragma once

#include <memory>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include <yats/identifier.h>
#include <yats/lambda_task.h>
#include <yats/task_configurator.h>
#include <yats/util.h>
#include <locale>

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
        
        std::ofstream file;
        file.open(filename, std::ios_base::out | std::ios_base::trunc);
        std::string input_ids;
        std::string output_ids;
        
        file << "digraph structs {" << std::endl;
        file << '\t' << "rankdir = LR;" << std::endl << std::endl;
        file << '\t' << "node [shape = record];" << std::endl;

        // Alle Knoten mit Inputs und Outputs erstellen
        // NODE_NAME [label = "NODE_NAME|{{<KEY1>INPUT1|<KEY2>INPUT2...}|{<KEY3>OUTPUT1|<KEY4>OUTPUT2...}}"];
        for (size_t i = 0; i < helpers.size(); ++i)
        {
            file << '\t' << "n" << i << "[label = \"" << "n" << i << "|{";
            file << '{' << inputs_to_string(*helpers[i]) << "}|";
            file << '{' << outputs_to_string(*helpers[i]) << '}';
            file << "}\"]" << std::endl;
        }

        file << std::endl;

        // Map output to helper index
        // outputs are unique
        std::map<const abstract_output_connector*, size_t> output_owner;
        std::set<const abstract_output_connector*> unused_outputs;
        for (size_t i = 0; i < m_tasks.size(); ++i)
        {
            auto outputs = helpers[i]->outputs();
            for (const auto output : outputs)
            {
                output_owner.emplace(output.first, i);
                unused_outputs.insert(output.first);
            }
        }
        
        int id_counter = 0;

        for (size_t i = 0; i < helpers.size(); ++i)
        {
            auto inputs = helpers[i]->inputs();
            for (const auto input : inputs)
            {
                auto source_location = input.first->output();
                
                // Input is not connected to an output
                if (source_location == nullptr)
                {
                    file << '\t' << "node [shape = point]; ";
                    file << 'u' << id_counter << ';' << std::endl;
                    file << '\t' << 'u' << id_counter << "->" << 'n' << i << ':' << "<i" << input.second << '>' << std::endl;
                    ++id_counter;
                }
                else
                {
                    const auto source_task_id = output_owner.at(source_location);
                    file << '\t' << 'n' << source_task_id << ':' << "<o" << helpers[source_task_id]->get_output_index(source_location) << "> -> " << 'n' << i << ':' << "<i" << input.second << '>' << std::endl;
                    unused_outputs.erase(source_location);
                }
            }
        }

        for (const auto& output : unused_outputs)
        {
            const auto helper_index = output_owner.at(output);
            file << '\t' << "node [shape = point]; ";
            file << 'u' << id_counter << ';' << std::endl;
            file << '\t' << 'n' << helper_index << ':' << "<o" << helpers[helper_index]->get_output_index(output) << '>' << "->" << 'u' << id_counter << std::endl;
            ++id_counter;
        }

        file << '}' << std::endl;

        file.close();

        // Als nächstes müssen die Verbindungen gesetzt werden:
        // ein zweiter loop über alle helper um die Verbindung anzulegen.
        // Wir haben inputs, die nicht von einem output geschrieben werden
        // Outputs, die keinen Input schreiben
        // und outputs, die in Inputs schreiben (normal).

    }

    static std::string inputs_to_string(abstract_connection_helper& helper)
    {
        std::stringstream input_id_stream;

        for (const auto& input : helper.inputs())
        {
            const auto index = input.second;
            input_id_stream << "<i" << index << ">" << yats::identifier::id_to_string(helper.get_input_id(index)) << "|";
        }
    
        auto tmp = input_id_stream.str();

        if (!tmp.empty())
        {
            tmp.pop_back();
        }

        return tmp;
    }

    static std::string outputs_to_string(abstract_connection_helper& helper)
    {
        std::stringstream output_id_stream;

        for (const auto& output : helper.outputs())
        {
            const auto index = output.second;
            output_id_stream << "<o" << index << ">" << yats::identifier::id_to_string(helper.get_output_id(index)) << "|";
        }

        auto tmp = output_id_stream.str();

        if (!tmp.empty())
        {
            tmp.pop_back();
        }

        return tmp;
    }

protected:
    std::vector<std::unique_ptr<abstract_task_configurator>> m_tasks;
};
}
