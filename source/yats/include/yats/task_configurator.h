#pragma once

#include <map>
#include <memory>

#include <yats/lambda_task.h>
#include <yats/task_container.h>
#include <yats/identifier.h>
#include <yats/util.h>

namespace yats
{

/**/
class abstract_task_configurator
{
public:
    abstract_task_configurator() = default;
    
    virtual ~abstract_task_configurator() = default;

    abstract_task_configurator(const abstract_task_configurator& other) = delete;
    abstract_task_configurator(abstract_task_configurator&& other) = delete;

    abstract_task_configurator& operator=(const abstract_task_configurator& other) = delete;
    abstract_task_configurator& operator=(abstract_task_configurator&& other) = delete;

    virtual std::unique_ptr<abstract_task_container> construct_task_container(std::unique_ptr<abstract_connection_helper> helper) const = 0;
    virtual std::unique_ptr<abstract_connection_helper> construct_connection_helper() const = 0;
};

template <typename Task, typename... Parameters>
class task_configurator : public abstract_task_configurator
{
    static_assert(has_run_v<Task>, "Can not create task_configurator because its task has no run function.");

public:
    using helper = decltype(make_helper(&Task::run));

    task_configurator(Parameters&&... parameters)
        : m_construction_parameters(std::forward<Parameters>(parameters)...)
    {
    }
    
    template <uint64_t Id>
    auto& input()
    {
        constexpr auto index = get_index_by_id_v<Id, typename helper::input_tuple>;
        using type = typename helper::input_connectors;
        return find<typename helper::input_tuple, std::tuple_element_t<index, type>>(m_inputs, Id);
    }

    template <uint64_t Id>
    auto& output()
    {
        constexpr auto index = get_index_by_id_v<Id, typename helper::output_tuple>;
        using type = typename helper::output_connectors;
        return find<typename helper::output_tuple, std::tuple_element_t<index, type>>(m_outputs, Id);
    }

    template <uint64_t Id, typename LambdaTask>
    void add_listener(LambdaTask task)
    {
        using type = decltype(make_lambda_task(&LambdaTask::operator()));
        constexpr auto index = get_index_by_id_v<Id, typename helper::output_tuple>;
        std::get<index>(m_listeners).push_back(typename type::function_type(task));
    }

    std::unique_ptr<abstract_task_container> construct_task_container(std::unique_ptr<abstract_connection_helper> helper) const override
    {
        return std::make_unique<task_container<Task, std::remove_reference_t<Parameters>...>>(static_cast<connection_helper<Task>*>(helper.get()), m_construction_parameters);
    }

    std::unique_ptr<abstract_connection_helper> construct_connection_helper() const override
    {
        return std::make_unique<connection_helper<Task>>(m_inputs, m_outputs, m_listeners);
    }

protected:
    template <typename IdTuple, typename Return, typename Parameter>
    Return& find(Parameter& tuple, uint64_t id)
    {
        auto connector = get<IdTuple, Return>(tuple, id);
        if (connector)
        {
            return *connector;
        }
        throw std::runtime_error("Id not found.");
    }

    template <typename IdTuple, typename Return, size_t Index = 0, typename Parameter = int>
    std::enable_if_t<(Index < std::tuple_size<IdTuple>::value), Return*> get(Parameter& tuple, uint64_t id)
    {
        auto elem = &std::get<Index>(tuple);
        if (id == std::tuple_element_t<Index, IdTuple>::id)
        {
            return elem;
        }
        return get<IdTuple, Return, Index + 1>(tuple, id);
    }

    template <typename IdTuple, typename Return, size_t Index = 0, typename Parameter = int>
    std::enable_if_t<Index == std::tuple_size<IdTuple>::value, Return*> get(Parameter&, uint64_t)
    {
        return nullptr;
    }

    typename helper::input_connectors m_inputs;
    typename helper::output_connectors m_outputs;
    typename helper::output_callbacks m_listeners;
    const std::tuple<std::remove_reference_t<Parameters>...> m_construction_parameters;
};
}
