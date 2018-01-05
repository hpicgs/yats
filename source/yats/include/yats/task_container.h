#pragma once

#include <algorithm>
#include <array>
#include <tuple>
#include <utility>

#include <yats/task_helper.h>

namespace yats
{

class abstract_task_container
{
public:
    abstract_task_container() = default;

    virtual ~abstract_task_container() = default;

    virtual void run() = 0;
    virtual bool can_run() const = 0;
};

template <typename Task>
class task_container : public abstract_task_container
{
public:
    using helper = decltype(make_helper(&Task::run));

    task_container(typename helper::input_queue_ptr input, typename helper::output_callbacks output)
        : m_input(std::move(input))
        , m_output(std::move(output))
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
    std::enable_if_t<!std::is_same<T, void>::value> invoke(std::integer_sequence<size_t, index...>)
    {
        auto output = m_task.run(get<index>()...);
        write(output);
    }

    template <size_t... index, typename T = typename helper::output_type>
    std::enable_if_t<std::is_same<T, void>::value> invoke(std::integer_sequence<size_t, index...>)
    {
        m_task.run(get<index>()...);
    }

    template <size_t index>
    auto get()
    {
        auto queue = std::get<index>(*m_input);
        auto value = queue.front();
        queue.pop();

        return value;
    }

    template <size_t index = 0, typename T = typename helper::output_type, typename Output = std::enable_if_t<std::is_same<T, void>::value, T>>
    std::enable_if_t<(index < helper::output_count)> write(Output& output)
    {
        auto& value = std::get<index>(output);
        for (auto& callback : std::get<index>(m_output))
        {
            callback(value);
        }

        write<index + 1>(output);
    }

    template <size_t index, typename T = typename helper::output_type, typename Output = std::enable_if_t<std::is_same<T, void>::value, T>>
    std::enable_if_t<index == helper::output_count> write(Output&)
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
