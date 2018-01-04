#pragma once

#include <map>
#include <memory>

#include <yats/input_connector.h>
#include <yats/output_connector.h>
#include <yats/task_container.h>

namespace yats
{

class abstract_connection_helper
{
public:
    template <typename Type>
    using locations = std::map<const Type*, size_t>;

    abstract_connection_helper(locations<abstract_input_connector> input, locations<abstract_output_connector> output)
        : m_in(input)
        , m_out(output)
    {
    }

    virtual ~abstract_connection_helper() = default;

    virtual void bind(const abstract_output_connector* connector, void* callback) = 0;
    virtual void* target(const abstract_input_connector* connector) = 0;

    const auto& inputs()
    {
        return m_in;
    }

    const auto& outputs()
    {
        return m_out;
    }

protected:
    template <typename LocationType, typename SequenceType, size_t... index>
    static locations<LocationType> map(const SequenceType& outputs, std::index_sequence<index...>)
    {
        // Prevent a warning about unused parameter when handling a run function with no parameters.
        (void) outputs;
        return { std::make_pair(&std::get<index>(outputs), index)... };
    }

    const locations<abstract_input_connector> m_in;
    const locations<abstract_output_connector> m_out;
};

template <typename Task>
class connection_helper : public abstract_connection_helper
{
public:
    using helper = decltype(make_helper(&Task::run));
    using input_sequence = std::make_index_sequence<std::tuple_size<typename helper::input_configuration>::value>;
    using output_sequence = std::make_index_sequence<std::tuple_size<typename helper::output_configuration>::value>;

    connection_helper(const typename helper::input_configuration& inputs, const typename helper::output_configuration& outputs)
        : abstract_connection_helper(map<abstract_input_connector>(inputs, input_sequence()), map<abstract_output_connector>(outputs, output_sequence()))
        , m_input(std::make_unique<typename helper::input_queue_base>())
        , m_callbacks(generate_callbacks(m_input, std::make_index_sequence<helper::parameter_count>()))
    {
    }

    void bind(const abstract_output_connector* connector, void* callback) override
    {
        auto location_id = m_out.at(connector);
        add(location_id, callback);
    }

    void* target(const abstract_input_connector* connector) override
    {
        auto location_id = m_in.at(connector);
        return get(location_id);
    }

    typename helper::input_queue queue()
    {
        return std::move(m_input);
    }

    typename helper::return_callbacks callbacks()
    {
        return std::move(m_output);
    }

protected:
    template <size_t... index>
    static typename helper::input_callbacks generate_callbacks(typename helper::input_queue& queue, std::integer_sequence<size_t, index...>)
    {
        // Prevent a warning about unused parameter when handling a run function with no parameters.
        (void) queue;
        return std::make_tuple(generate_callback<index>(queue)...);
    }

    template <size_t index>
    static typename std::tuple_element_t<index, typename helper::input_callbacks> generate_callback(typename helper::input_queue& queue)
    {
        using parameter_type = typename std::tuple_element_t<index, typename helper::input_queue_base>::value_type;
        return [&current = std::get<index>(*queue)](parameter_type input) mutable
        {
            current.push(input);
        };
    }

    template <size_t index = 0>
    std::enable_if_t<(index < helper::output_parameter_count)> add(size_t location_id, void* raw_callback)
    {
        if (index == location_id)
        {
            using parameter = std::tuple_element_t<index, typename helper::return_base>;
            auto callback = static_cast<std::function<void(parameter)>*>(raw_callback);
            std::get<index>(m_output).push_back(*callback);
        }
        else
        {
            add<index + 1>(location_id, raw_callback);
        }
    }

    template <size_t index = 0>
    std::enable_if_t<index == helper::output_parameter_count> add(size_t, void*)
    {
        throw std::runtime_error("Output Parameter locationId not found.");
    }

    template <size_t index = 0>
    std::enable_if_t<(index < helper::parameter_count), void*> get(size_t location_id)
    {
        if (index == location_id)
        {
            return &std::get<index>(m_callbacks);
        }
        else
        {
            return get<index + 1>(location_id);
        }
    }

    template <size_t index = 0>
    std::enable_if_t<index == helper::parameter_count, void*> get(size_t)
    {
        throw std::runtime_error("Input Parameter locationId not found.");
    }

    typename helper::input_queue m_input;
    typename helper::return_callbacks m_output;
    typename helper::input_callbacks m_callbacks;
};
}
