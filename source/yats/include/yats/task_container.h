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

    task_container(connection_helper<Task>* connection, std::tuple<Parameters...> parameter_tuple)
        : abstract_task_container(connection->following_nodes())
        , m_input(connection->queue())
        , m_output(connection->callbacks())
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
        //const auto& v = *m_input;
        //auto& queue = std::get<index>(v);
        //decltype(queue.front()) value;
        //std::swap(value, queue.front());
        //auto value = std::move(queue.front());
        //queue.pop();
        auto value = std::move(std::get<index>(*m_input).front());
        std::get<index>(*m_input).pop();

        return value;
    }

    template <typename SlotType, typename ValueType = typename SlotType::value_type>
    std::enable_if_t<std::is_copy_constructible<ValueType>::value, ValueType> copy_value(SlotType& value)
    {
        return value.clone();
    }

    template <typename SlotType, typename ValueType = typename SlotType::value_type>
    std::enable_if_t<!std::is_copy_constructible<ValueType>::value, ValueType> copy_value(SlotType&)
    {
        throw std::runtime_error("An not copyable type cannot be used in multiple connections.");
    }

    template <size_t index = 0, typename T = typename helper::output_type, typename Output = std::enable_if_t<std::is_same<T, void>::value, T>>
    std::enable_if_t<(index < helper::output_count)> write(Output output)
    {
        auto& value = std::get<index>(output);
        auto following_nodes = std::get<index>(m_output);
        for (size_t i = 1; i <= following_nodes.size(); ++i)
        {
            auto& callback = following_nodes[i - 1];
            if (i == following_nodes.size())
            {
                // Move the last value.
                callback(value.extract());
            }
            else
            {
                // Copy everything else.
                callback(copy_value(value));
            }
        }

        write<index + 1>(std::move(output));
    }

    template <size_t index, typename T = typename helper::output_type, typename Output = std::enable_if_t<std::is_same<T, void>::value, T>>
    std::enable_if_t<index == helper::output_count> write(Output)
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

    typename helper::input_queue_ptr m_input;
    typename helper::output_callbacks m_output;
    Task m_task;
};
}
