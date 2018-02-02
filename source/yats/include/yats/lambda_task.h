#pragma once

#include <functional>

namespace yats
{

template <typename Return, typename... Parameters>
class lambda_task
{
public:
    using function_type = std::function<Return(Parameters...)>;

    lambda_task(std::function<Return(Parameters...)> callback)
        : m_run(callback)
    {
    }

    Return run(Parameters... parameters)
    {
        return m_run(std::move(parameters)...);
    }

protected:
    function_type m_run;
};

template <typename Return, typename Task, typename... Parameters>
static constexpr lambda_task<Return, Parameters...> make_lambda_task(Return (Task::*)(Parameters...));

template <typename Return, typename Task, typename... Parameters>
static constexpr lambda_task<Return, Parameters...> make_lambda_task(Return (Task::*)(Parameters...) const);
}
