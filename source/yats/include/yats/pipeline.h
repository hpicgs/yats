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

class io_iterator
{
public:
    explicit io_iterator(const std::vector<std::unique_ptr<abstract_task_configurator>>* tasks)
        : m_helper_index(std::numeric_limits<size_t>::max()), m_tasks(tasks), m_iterator_changed(false)
    {
        // required to gain access to the input and output connectors
        for (const auto& configurator : *m_tasks)
        {
            m_helpers.emplace_back(configurator->construct_connection_helper());
        }

        for (size_t i = 0; i < m_tasks->size(); ++i)
        {
            auto outputs = m_helpers[i]->outputs();
            for (const auto output : outputs)
            {
                m_output_owners.emplace(output.first, i);
            }
        }
    }

    bool next()
    {
        if (advance_to_next_input())
        {
            return true;
        }

        while (advance_to_next_helper())
        {
            if (advance_to_next_input())
            {
                return true;
            }
        }

        return false;
    }

    const abstract_input_connector* input() const
    {
        return m_input_iterator->first;
    }

    size_t index_of_input() const
    {
        return m_input_iterator->second;
    }

    size_t source_index() const
    {
        if (source() != nullptr)
        {
            return get_helper_index_to_output(source());
        }
        return std::numeric_limits<size_t>::max();
    }

    const abstract_output_connector* source() const
    {
        return input()->output();
    }

    abstract_connection_helper* get_helper() const
    {
        return m_helpers[m_helper_index].get();
    }

    abstract_connection_helper* get_helper(size_t index) const
    {
        return m_helpers[index].get();
    }

    std::vector<std::unique_ptr<abstract_connection_helper>> take_helpers()
    {
        return std::move(m_helpers);
    }

    size_t helper_index() const
    {
        return m_helper_index;
    }

    size_t get_helper_index_to_output(const abstract_output_connector* output) const
    {
        return m_output_owners.at(output);
    }

    size_t helper_count() const
    {
        return m_helpers.size();
    }

protected:
    size_t m_helper_index;
    const std::vector<std::unique_ptr<abstract_task_configurator>>* m_tasks;
    std::map<const abstract_input_connector*, size_t>::iterator m_input_iterator;
    std::map<const abstract_input_connector*, size_t> m_inputs;
    std::vector<std::unique_ptr<abstract_connection_helper>> m_helpers;
    std::map<const abstract_output_connector*, size_t> m_output_owners;
    bool m_iterator_changed;

    bool advance_to_next_helper()
    {
        ++m_helper_index;
        if (m_helper_index >= m_helpers.size())
        {
            return false;
        }
        m_inputs = m_helpers[m_helper_index]->inputs();
        m_input_iterator = m_inputs.begin();
        m_iterator_changed = true;
        return true;
    }

    bool advance_to_next_input()
    {
        if (m_helper_index >= m_helpers.size())
        {
            return false;
        }

        if (m_iterator_changed)
        {
            m_iterator_changed = false;
        }
        else
        {
            ++m_input_iterator;
        }
        
        return m_input_iterator != m_inputs.end();
    }
};

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
        io_iterator iter(&m_tasks);
        
        while(iter.next())
        {
            auto helper = iter.get_helper(iter.source_index());
            helper->bind(iter.source(), iter.get_helper()->target(iter.input()));
            helper->add_following(iter.helper_index());
        }

        std::vector<std::unique_ptr<abstract_task_container>> tasks;
        auto helpers = iter.take_helpers();
        for (size_t i = 0; i < m_tasks.size(); ++i)
        {
            tasks.push_back(m_tasks[i]->construct_task_container(std::move(helpers[i])));
        }

        return tasks;
    }

    /**
     * Saves the pipeline to a file in the Graphviz DOT format.
     * @param filename Filename to save the pipeline to.
     */
    void save_to_file(const std::string& filename)
    {
        io_iterator iter(&m_tasks);

        std::ofstream file;
        file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        file.open(filename, std::ios_base::trunc);
        
        // writes first part of the body for the DOT file. 
        file << "digraph structs {" << std::endl;
        file << '\t' << "rankdir = LR;" << std::endl << std::endl;
        file << '\t' << "node [shape = record];" << std::endl;

        // Creates a vertex for each task including the task's inputs and outputs
        // NODE_NAME [label = "NODE_NAME|{{<KEY1>INPUT1|<KEY2>INPUT2...}|{<KEY3>OUTPUT1|<KEY4>OUTPUT2...}}"];
        for (size_t i = 0; i < iter.helper_count(); ++i)
        {
            file << '\t' << "n" << i << "[label = \"" << "n" << i << "|{";
            file << '{' << inputs_to_string(*iter.get_helper(i)) << "}|";
            file << '{' << outputs_to_string(*iter.get_helper(i)) << '}';
            file << "}\"]" << std::endl;
        }

        file << std::endl;

        std::set<const abstract_output_connector*> unused_outputs;
        for (size_t i = 0; i < m_tasks.size(); ++i)
        {
            auto outputs = iter.get_helper(i)->outputs();
            for (const auto output : outputs)
            {
                unused_outputs.insert(output.first);
            }
        }
        
        auto id_counter = 0;

        while (iter.next())
        {
            // Input is not connected to an output
            if (iter.source() == nullptr)
            {
                file << '\t' << "node [shape = point]; ";
                file << 'u' << id_counter << ';' << std::endl;
                file << '\t' << 'u' << id_counter << "->" << 'n' << iter.helper_index() << ':' << "<i" << iter.index_of_input() << '>' << std::endl;
                ++id_counter;
            }
            else
            {
                file << '\t' << 'n' << iter.source_index() << ':' << "<o" << iter.get_helper(iter.source_index())->get_output_index(iter.source());
                file << "> -> " << 'n' << iter.helper_index() << ':' << "<i" << iter.index_of_input() << '>' << std::endl;
                unused_outputs.erase(iter.source());
            }
        }

        for (const auto& output : unused_outputs)
        {
            const auto helper_index = iter.get_helper_index_to_output(output);
            file << '\t' << "node [shape = point]; ";
            file << 'u' << id_counter << ';' << std::endl;
            file << '\t' << 'n' << helper_index << ':' << "<o" << iter.get_helper(helper_index)->get_output_index(output);
            file << '>' << "->" << 'u' << id_counter << std::endl;
            ++id_counter;
        }

        file << '}' << std::endl;

        file.close();
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
