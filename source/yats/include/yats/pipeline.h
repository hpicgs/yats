#pragma once

#include <memory>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include <yats/identifier.h>
#include <yats/constraint_helper.h>
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
    std::vector<std::unique_ptr<abstract_task_container>> build(const std::function<void(abstract_task_container*)>& external_callback) const &&
    {
        auto helpers = get_helpers();
        auto output_owners = get_output_owners(helpers);

        for (size_t i = 0; i < helpers.size(); ++i)
        {
            auto inputs = helpers[i]->inputs();
            for (const auto input : inputs)
            {
                if (m_tasks[i]->is_external(input.first))
                {
                    continue;
                }
                auto source_location = input.first->output();
                const auto source_task_id = output_owners.at(source_location);

                // connect output to input
                helpers[source_task_id]->bind(source_location, helpers[i]->target(input.first));
                // tasks connected to an output are successors 
                helpers[source_task_id]->add_following(i);
            }
        }

        std::vector<std::unique_ptr<abstract_task_container>> tasks;
        auto constraint_map = thread_group_helper::map_thread_groups(m_tasks);
        for (size_t i = 0; i < m_tasks.size(); ++i)
        {
            tasks.push_back(m_tasks[i]->construct_task_container(std::move(helpers[i]), external_callback));

            std::vector<size_t> constraints;
            for (const auto& constraint_name : m_tasks[i]->thread_constraints().names())
            {
                constraints.push_back(constraint_map.at(constraint_name));
            }
            tasks.back()->set_constraints(constraints);
        }

        return tasks;
    }

    /**
     * Saves the pipeline to a file in the Graphviz DOT format.
     * @param filename Filename to save the pipeline to.
     */
    void save_to_file(const std::string& filename) const
    {
        std::ofstream file;
        file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        file.open(filename, std::ios_base::trunc);
        std::string input_ids;
        std::string output_ids;

        constexpr auto line_break = '\n';

        // writes first part of the body for the DOT file. 
        file << "digraph structs {" << line_break;
        file << '\t' << "rankdir = LR;" << line_break << line_break;
        file << '\t' << "node [shape = record];" << line_break;

        auto helpers = get_helpers();
        auto output_owners = get_output_owners(helpers);

        // Creates a vertex for each task including the task's inputs and outputs.
        // The tasks are named n0, n1, ... according to their helper index. They are labeled with their class name.
        // Key for input values are i1, i2, .... Keys for out values are o1, o2, ... .
        // NODE_NAME [label = "NODE_NAME|{{<KEY1>INPUT1|<KEY2>INPUT2...}|{<KEY3>OUTPUT1|<KEY4>OUTPUT2...}}"];
        for (size_t i = 0; i < helpers.size(); ++i)
        {
            file << '\t' << "n" << i << "[label = \"" << excape_xml_entities(helpers[i]->task_name()) << "|{";
            file << '{' << inputs_to_string(*helpers[i]) << "}|";
            file << '{' << outputs_to_string(*helpers[i]) << '}';
            file << "}\"]" << line_break;
        }

        file << line_break;

        // We need to keep track of outputs which are not connected.
        std::set<const abstract_output_connector*> unused_outputs;
        for (size_t i = 0; i < m_tasks.size(); ++i)
        {
            auto outputs = helpers[i]->outputs();
            for (const auto output : outputs)
            {
                unused_outputs.insert(output.first);
            }
        }

        // Counter to name either inputs not fed by an output or outputs not connected by an
        // input.
        auto id_counter = 0;

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
                    file << 'u' << id_counter << ';' << line_break;
                    file << '\t' << 'u' << id_counter << "->" << 'n' << i << ':' << "<i" << input.second << '>' << line_break;
                    ++id_counter;
                } // normal case. Adds edge from the output connected to the current input.
                else
                {
                    const auto source_task_id = output_owners.at(source_location);
                    file << '\t' << 'n' << source_task_id << ':' << "<o" << helpers[source_task_id]->output_index(source_location) << "> -> " << 'n' << i << ':' << "<i" << input.second << '>' << line_break;
                    unused_outputs.erase(source_location);
                }
            }
        }

        // Adds outputs which are not connected to an input.
        for (const auto& output : unused_outputs)
        {
            const auto helper_index = output_owners.at(output);
            file << '\t' << "node [shape = point]; ";
            file << 'u' << id_counter << ';' << line_break;
            file << '\t' << 'n' << helper_index << ':' << "<o" << helpers[helper_index]->output_index(output) << '>' << "->" << 'u' << id_counter << line_break;
            ++id_counter;
        }

        file << '}' << line_break;

        file.close();
    }
protected:
    std::vector<std::unique_ptr<abstract_task_configurator>> m_tasks;

    /**
     * Creates a connection helper for each task. This is required to gain access
     * to the input and output connectors of each task.
     * @return Vector of connection helpers 
     */
    std::vector<std::unique_ptr<abstract_connection_helper>> get_helpers() const
    {
        // required to gain access to the input and output connectors
        std::vector<std::unique_ptr<abstract_connection_helper>> helpers;
        for (const auto& configurator : m_tasks)
        {
            helpers.emplace_back(configurator->construct_connection_helper());
        }

        return helpers;
    }

    /**
     * Maps every output in {@code helpers} to the corresponding helper index in {@code helpers}.
     * @param helpers Vector of helpers
     * @return A map, which contains the output_connector as key and the index of the helper in {@code helpers} as value.
     */
    std::map<const abstract_output_connector*, size_t> get_output_owners(const std::vector<std::unique_ptr<abstract_connection_helper>>& helpers) const
    {
        // Map output to helper index
        // outputs are unique
        std::map<const abstract_output_connector*, size_t> output_owners;
        for (size_t i = 0; i < m_tasks.size(); ++i)
        {
            auto outputs = helpers[i]->outputs();
            for (const auto output : outputs)
            {
                output_owners.emplace(output.first, i);
            }
        }
        return output_owners;
    }

    /**
    * Creates a string of all inputs in {@code helper} of the format <KEY1>INPUT1|<KEY2>INPUT2...
    * Keys are of the format <i0>, <i1>, etc.
    * The order of the inputs depends on their placement in the underlying map in {@code helper}.
    * @param helper Helper whose inputs are to by converted to a string.
    * @return A string containing all inputs according to the format described above
    */
    static std::string inputs_to_string(abstract_connection_helper& helper)
    {
        std::stringstream input_id_stream;
        const io_id_helper io_id_helper = helper.get_io_id_helper();

        for (const auto& input : helper.inputs())
        {
            const auto index = input.second;
            input_id_stream << "<i" << index << ">" << yats::identifier::id_to_string(io_id_helper.input_id(index)) << "|";
        }

        auto tmp = input_id_stream.str();

        if (!tmp.empty())
        {
            tmp.pop_back();
        }

        return tmp;
    }

    /**
    * Creates a string of all outputs in {@code helper} of the format <KEY1>OUTPUT1|<KEY2>OUTPUT2...
    * Keys are of the format <o0>, <o1>, etc.
    * The order of the inputs depends on their placement in the underlying map in {@code helper}.
    * @param helper Helper whose outputs are to by converted to a string.
    * @return A string containing all outputs according to the format described above
    */
    static std::string outputs_to_string(abstract_connection_helper& helper)
    {
        std::stringstream output_id_stream;
        const io_id_helper io_id_helper = helper.get_io_id_helper();

        for (const auto& output : helper.outputs())
        {
            const auto index = output.second;
            output_id_stream << "<o" << index << ">" << yats::identifier::id_to_string(io_id_helper.output_id(index)) << "|";
        }

        auto tmp = output_id_stream.str();

        if (!tmp.empty())
        {
            tmp.pop_back();
        }

        return tmp;
    }

    /**
     * Escapes all xml entities &lt;, &gt;, &amp;, &quot; and &apos; in <@code str>.
     * @param str String with entities to be escaped.
     * @return String, where the above named entities have been escaped
     */
    static std::string excape_xml_entities(const std::string& str)
    {
        std::string escaped_string;

        for (const auto& c : str)
        {
            switch (c)
            {
            case '<':
                escaped_string.append("&lt;");
                break;
            case '>':
                escaped_string.append("&gt;");
                break;
            case '&':
                escaped_string.append("&amp;");
                break;
            case '"':
                escaped_string.append("&quot;");
                break;
            case '\'':
                escaped_string.append("&apos;");
                break;
            default:
                escaped_string.append(1, c);
            }
        }
        return escaped_string;
    }
};
}
