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
        auto copyable = check_copyable(std::make_index_sequence<helper::output_count>());
        if (!copyable)
        {
            throw std::runtime_error("A not copyable type cannot be used in multiple connections.");
        }
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
    template <size_t... Index, typename T = typename helper::output_type>
    std::enable_if_t<is_tuple_v<T>> invoke(std::integer_sequence<size_t, Index...>)
    {
        write(m_task.run(get<Index>()...));
    }

    template <size_t... index, typename T = typename helper::output_type>
    std::enable_if_t<!std::is_same<T, void>::value && !is_tuple_v<T>> invoke(std::integer_sequence<size_t, index...>)
    {
        write(std::make_tuple(m_task.run(get<index>()...)));
    }

    template <size_t... Index, typename T = typename helper::output_type>
    std::enable_if_t<std::is_same<T, void>::value> invoke(std::integer_sequence<size_t, Index...>)
    {
        m_task.run(get<Index>()...);
    }

    /**
     * Extracts (removes) first value from input queue and returns it.
     * @param <Index> Index of the input to use.
     */
    template <size_t Index>
    auto get()
    {
        return std::get<Index>(*m_input).extract();
    }

    template <typename SlotType, typename ValueType = typename SlotType::value_type>
    static std::enable_if_t<std::is_copy_constructible<ValueType>::value, ValueType> copy_value(const SlotType& value)
    {
        return value.clone();
    }

    template <typename SlotType, typename ValueType = typename SlotType::value_type>
    static std::enable_if_t<!std::is_copy_constructible<ValueType>::value, ValueType> copy_value(const SlotType&)
    {
        throw std::runtime_error("If this gets thrown, the library is broken.");
    }

    /**
     * Writes value of output into the inputs of the following tasks
     * @param output The output used to pass values to following inputs.
     */
    template <size_t Index = 0, typename Output = typename helper::output_type>
    std::enable_if_t<(Index < helper::output_count)> write(Output output)
    {
        // Contains the callbacks to write into inputs of the following tasks.
        const auto& callbacks = std::get<Index>(m_output);
        auto& slot = std::get<Index>(output);

        // cend() - 1 and back() will fail for empty callbacks
        if (!callbacks.empty())
        {
            // Copy the value before we move it the last time we need it.
            for (auto it = callbacks.cbegin(); it != callbacks.cend() - 1; ++it)
            {
                (*it)(copy_value(slot));
            }
            callbacks.back()(slot.extract());
        }

        write<Index + 1>(std::move(output));
    }

    template <size_t Index, typename Output = typename helper::output_type>
    std::enable_if_t<Index == helper::output_count> write(Output)
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

    template <size_t... Index>
    bool check_copyable(std::integer_sequence<size_t, Index...>) const
    {
        std::array<bool, sizeof...(Index)> copyable{ { check_copyable_impl<Index>()... } };
        return std::all_of(copyable.cbegin(), copyable.cend(), [](bool input) { return input; });
    }

    template <size_t Index>
    bool check_copyable_impl() const
    {
        return std::is_copy_constructible<std::tuple_element_t<Index, typename helper::output_tuple>>::value || std::get<Index>(m_output).size() < 2;
    }

    typename helper::input_queue_ptr m_input;
    typename helper::output_callbacks m_output;
    Task m_task;
};
}
