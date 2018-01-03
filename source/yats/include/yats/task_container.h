#pragma once

#include <algorithm>
#include <array>
#include <tuple>
#include <utility>

#include <yats/util.h>

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
    using Helper = decltype(MakeHelper(&Task::run));

    task_container(typename Helper::InputQueue input, typename Helper::ReturnCallbacks output)
        : m_input(std::move(input))
        , m_output(std::move(output))
    {
    }

    void run() override
    {
        invoke(std::make_index_sequence<Helper::ParameterCount>());
    }

    bool can_run() const override
    {
        return can_run_impl(std::make_index_sequence<Helper::ParameterCount>());
    }

protected:
    template <size_t... index, typename T = typename Helper::ReturnType>
    std::enable_if_t<!std::is_same<T, void>::value> invoke(std::integer_sequence<size_t, index...>)
    {
        auto output = m_task.run(get<index>()...);
        write(output);
    }

    template <size_t... index, typename T = typename Helper::ReturnType>
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

    template <size_t index = 0, typename T = typename Helper::ReturnType, typename Output = std::enable_if_t<std::is_same<T, void>::value, T>>
    std::enable_if_t<(index < Helper::OutputParameterCount)> write(Output& output)
    {
        auto& value = std::get<index>(output);
        for (auto& callback : std::get<index>(m_output))
        {
            callback(value);
        }

        write<index + 1>(output);
    }

    template <size_t index, typename T = typename Helper::ReturnType, typename Output = std::enable_if_t<std::is_same<T, void>::value, T>>
    std::enable_if_t<index == Helper::OutputParameterCount> write(Output&)
    {
    }

    template <size_t... Index, size_t InputCount = Helper::ParameterCount>
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

    typename Helper::InputQueue m_input;
    typename Helper::ReturnCallbacks m_output;
    Task m_task;
};
}
