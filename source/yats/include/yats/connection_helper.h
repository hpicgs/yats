#pragma once

#include <map>
#include <memory>

#include <yats/input_connector.h>
#include <yats/output_connector.h>
#include <yats/task_container.h>

namespace yats
{

class AbstractConnectionHelper
{
public:
    template <typename Type>
    using Locations = std::map<const Type*, size_t>;

    AbstractConnectionHelper(Locations<AbstractInputConnector> input, Locations<AbstractOutputConnector> output)
        : m_in(input)
        , m_out(output)
    {
    }

    virtual ~AbstractConnectionHelper() = default;

    virtual void bind(const AbstractOutputConnector* connector, void* callback) = 0;
    virtual void* target(const AbstractInputConnector* connector) = 0;

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
    static Locations<LocationType> map(const SequenceType& outputs, std::index_sequence<index...>)
    {
        // Prevent a warning about unused parameter when handling a run function with no parameters.
        (void) outputs;
        return { std::make_pair(&std::get<index>(outputs), index)... };
    }

    const Locations<AbstractInputConnector> m_in;
    const Locations<AbstractOutputConnector> m_out;
};

template <typename Task>
class ConnectionHelper : public AbstractConnectionHelper
{
public:
    using Helper = decltype(MakeHelper(&Task::run));
    using InputSequence = std::make_index_sequence<std::tuple_size<typename Helper::InputConfiguration>::value>;
    using OutputSequence = std::make_index_sequence<std::tuple_size<typename Helper::OutputConfiguration>::value>;

    ConnectionHelper(const typename Helper::InputConfiguration& inputs, const typename Helper::OutputConfiguration& outputs)
        : AbstractConnectionHelper(map<AbstractInputConnector>(inputs, InputSequence()), map<AbstractOutputConnector>(outputs, OutputSequence()))
        , m_input(std::make_unique<typename Helper::InputQueueBase>())
        , m_callbacks(generateCallbacks(m_input, std::make_index_sequence<Helper::ParameterCount>()))
    {
    }

    void bind(const AbstractOutputConnector* connector, void* callback) override
    {
        auto locationId = m_out.at(connector);
        add(locationId, callback);
    }

    void* target(const AbstractInputConnector* connector) override
    {
        auto locationId = m_in.at(connector);
        return get(locationId);
    }

    typename Helper::InputQueue queue()
    {
        return std::move(m_input);
    }

    typename Helper::ReturnCallbacks callbacks()
    {
        return std::move(m_output);
    }

protected:
    template <size_t... index>
    static typename Helper::InputCallbacks generateCallbacks(typename Helper::InputQueue& queue, std::integer_sequence<size_t, index...>)
    {
        // Prevent a warning about unused parameter when handling a run function with no parameters.
        (void) queue;
        return std::make_tuple(generateCallback<index>(queue)...);
    }

    template <size_t index>
    static typename std::tuple_element_t<index, typename Helper::InputCallbacks> generateCallback(typename Helper::InputQueue& queue)
    {
        using ParameterType = typename std::tuple_element_t<index, typename Helper::InputQueueBase>::value_type;
        return [&current = std::get<index>(*queue)](ParameterType input) mutable
        {
            current.push(input);
        };
    }

    template <size_t index = 0>
        std::enable_if_t < index<Helper::OutputParameterCount> add(size_t locationId, void* rawCallback)
    {
        if (index == locationId)
        {
            using Parameter = std::tuple_element_t<index, typename Helper::ReturnBase>;
            auto callback = static_cast<std::function<void(Parameter)>*>(rawCallback);
            std::get<index>(m_output).push_back(*callback);
        }
        else
        {
            add<index + 1>(locationId, rawCallback);
        }
    }

    template <size_t index = 0>
    std::enable_if_t<index == Helper::OutputParameterCount> add(size_t, void*)
    {
        throw std::runtime_error("Output Parameter locationId not found.");
    }

    template <size_t index = 0>
        std::enable_if_t < index<Helper::ParameterCount, void*> get(size_t locationId)
    {
        if (index == locationId)
        {
            using Parameter = std::tuple_element_t<index, typename Helper::Input>;
            return &std::get<index>(m_callbacks);
        }
        else
        {
            return get<index + 1>(locationId);
        }
    }

    template <size_t index = 0>
    std::enable_if_t<index == Helper::ParameterCount, void*> get(size_t)
    {
        throw std::runtime_error("Input Parameter locationId not found.");
    }

    typename Helper::InputQueue m_input;
    typename Helper::ReturnCallbacks m_output;
    typename Helper::InputCallbacks m_callbacks;
};
}
