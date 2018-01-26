#pragma once

#include <algorithm>
#include <array>
#include <stdexcept>
#include <tuple>
#include <utility>
#include <vector>

#include <yats/connection_helper.h>
#include <yats/task_helper.h>
#include <yats/util.h>

namespace yats
{

class abstract_task_container
{
public:
    abstract_task_container(const std::set<size_t>& following_nodes)
        : m_following_nodes(following_nodes.cbegin(), following_nodes.cend())
    {
    }

    virtual ~abstract_task_container() = default;

    abstract_task_container(const abstract_task_container& other) = delete;
    abstract_task_container(abstract_task_container&& other) = delete;

    abstract_task_container& operator=(const abstract_task_container& other) = delete;
    abstract_task_container& operator=(abstract_task_container&& other) = delete;

    virtual void run() = 0;
    virtual bool can_run() const = 0;

    const std::vector<size_t>& following_nodes()
    {
        return m_following_nodes;
    }

protected:
    const std::vector<size_t> m_following_nodes;
};

template <typename Task, typename... Parameters>
class task_container : public abstract_task_container
{
public:
    using helper = decltype(make_helper(&Task::run));

    task_container(connection_helper<Task>* connection, std::tuple<Parameters...> parameter_tuple, typename helper::output_callbacks listeners)
        : abstract_task_container(connection->following_nodes())
        , m_input(connection->queue())
        , m_output(connection->callbacks())
        , m_listeners(std::move(listeners))
        , m_task(make_from_tuple<Task>(std::move(parameter_tuple)))
    {
    }

    void run() override
    {
        invoke(std::make_index_sequence<helper::input_count>());
    }

    bool can_run() const override
    {
        return can_run_impl(std::make_index_sequence<helper::input_count>());
    }

protected:
    template <size_t... index, typename T = typename helper::output_type>
    std::enable_if_t<is_tuple_v<T>> invoke(std::integer_sequence<size_t, index...>)
    {
        write(m_task.run(get<index>()...));
    }

    template <size_t... index, typename T = typename helper::output_type>
    std::enable_if_t<!std::is_same<T, void>::value && !is_tuple_v<T>> invoke(std::integer_sequence<size_t, index...>)
    {
        write(std::make_tuple(m_task.run(get<index>()...)));
    }

    template <size_t... index, typename T = typename helper::output_type>
    std::enable_if_t<std::is_same<T, void>::value> invoke(std::integer_sequence<size_t, index...>)
    {
        m_task.run(get<index>()...);
    }

    template <size_t index>
    auto get()
    {
        auto& queue = std::get<index>(*m_input);
        auto value = std::move(queue.front());
        queue.pop();

        return value;
    }

    template <typename SlotType, typename ValueType = typename SlotType::value_type>
    static std::enable_if_t<std::is_copy_constructible<ValueType>::value, ValueType> copy_value(const SlotType& value)
    {
        return value.clone();
    }

    template <typename SlotType, typename ValueType = typename SlotType::value_type>
    static std::enable_if_t<!std::is_copy_constructible<ValueType>::value, ValueType> copy_value(const SlotType&)
    {
        throw std::runtime_error("A not copyable type cannot be used in multiple connections.");
    }

    template <size_t Index = 0, typename T = typename helper::output_type>
    std::enable_if_t<(Index < helper::output_count)> write(T output)
    {
        auto& slot = std::get<Index>(output);

        // Contains the callbacks which write the value into the following queues.
        const auto& following_nodes = std::get<Index>(m_output);

        // Copy everything but the last.
        for (size_t i = 0; i < following_nodes.size() - 1; ++i)
        {
            following_nodes[i](copy_value(slot));
        }

        // Move the last value.
        const auto& callback = following_nodes.back();
        if (std::get<Index>(m_listeners).size() == 0)
        {
            callback(slot.extract());
        }
        else
        {
            callback(copy_value(slot));
            notify_listeners<Index>(std::move(slot));
        }

        write<Index + 1>(std::move(output));
    }

    template <size_t index, typename T = typename helper::output_type>
    std::enable_if_t<index == helper::output_count> write(T)
    {
    }

    template <size_t... Index, size_t InputCount = helper::input_count>
    bool can_run_impl(std::integer_sequence<size_t, Index...>) const
    {
        std::array<bool, sizeof...(Index)> has_inputs{ { check_input<Index>()... } };
        return std::all_of(has_inputs.cbegin(), has_inputs.cend(), [](bool input) { return input; });
    }

    template <size_t Index>
    bool check_input() const
    {
        return std::get<Index>(*m_input).size() > 0;
    }

    template <size_t Index>
    void notify_listeners(std::tuple_element_t<Index, typename helper::output_tuple> slot) const
    {
        const auto& listeners = std::get<Index>(m_listeners);
        for (size_t i = 0; i < listeners.size() - 1; ++i)
        {
            listeners[i](copy_value(slot));
        }
        listeners.back()(slot.extract());
    }

    typename helper::input_queue_ptr m_input;
    typename helper::output_callbacks m_output;
    typename helper::output_callbacks m_listeners;
    Task m_task;
};
}
