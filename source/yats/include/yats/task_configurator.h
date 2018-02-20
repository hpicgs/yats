#pragma once

#include <memory>
#include <set>

#include <yats/lambda_task.h>
#include <yats/task_container.h>
#include <yats/identifier.h>
#include <yats/util.h>

namespace yats
{

class abstract_task_configurator
{
public:
    abstract_task_configurator() = default;
    
    virtual ~abstract_task_configurator() = default;

    abstract_task_configurator(const abstract_task_configurator& other) = delete;
    abstract_task_configurator(abstract_task_configurator&& other) = delete;

    abstract_task_configurator& operator=(const abstract_task_configurator& other) = delete;
    abstract_task_configurator& operator=(abstract_task_configurator&& other) = delete;

    virtual std::unique_ptr<abstract_task_container> construct_task_container(std::unique_ptr<abstract_connection_helper> helper, const std::function<void(abstract_task_container*)>& external_callback) = 0;
    virtual std::unique_ptr<abstract_connection_helper> construct_connection_helper() const = 0;

    bool is_external(const abstract_input_connector* connector) const
    {
        return m_externals.find(connector) != m_externals.cend();
    }

protected:
    void mark_as_external(const abstract_input_connector* connector)
    {
        m_externals.insert(connector);
    }

    std::set<const abstract_input_connector*> m_externals;
};

template <typename Task, typename... Parameters>
class task_configurator : public abstract_task_configurator
{
    static_assert(has_run_v<Task>, "Can not create task_configurator because its task has no run function.");

    using helper = decltype(make_helper(&Task::run));
    using input_connectors = typename helper::input_connectors;
    using input_tuple = typename helper::input_tuple;
    using input_writers = typename helper::input_writers;
    using input_writers_ptr = typename helper::input_writers_ptr;
    using output_callbacks = typename helper::output_callbacks;
    using output_connectors = typename helper::output_connectors;
    using output_tuple = typename helper::output_tuple;

public:
    task_configurator(Parameters&&... parameters)
        : m_writers(std::make_unique<input_writers>())
        , m_construction_parameters(std::forward<Parameters>(parameters)...)
    {
    }
    
    template <uint64_t Id>
    auto& input()
    {
        constexpr auto index = get_index_by_id_v<Id, input_tuple>;
        return find<input_tuple, std::tuple_element_t<index, input_connectors>>(m_inputs, Id);
    }

    template <uint64_t Id>
    auto& output()
    {
        constexpr auto index = get_index_by_id_v<Id, output_tuple>;
        return find<output_tuple, std::tuple_element_t<index, output_connectors>>(m_outputs, Id);
    }

    template <uint64_t Id>
    const auto& mark_as_external()
    {
        abstract_task_configurator::mark_as_external(&input<Id>());
        constexpr auto index = get_index_by_id_v<Id, input_tuple>;
        return std::get<index>(*m_writers).external_function;
    }

    template <uint64_t Id, typename Callable>
    void add_listener(Callable callable)
    {
        using type = decltype(make_lambda_task(&Callable::operator()));
        constexpr auto index = get_index_by_id_v<Id, output_tuple>;
        std::get<index>(m_listeners).push_back(typename type::function_type(std::move(callable)));
    }

    std::unique_ptr<abstract_task_container> construct_task_container(std::unique_ptr<abstract_connection_helper> helper, const std::function<void(abstract_task_container*)>& external_callback) override
    {
        return std::make_unique<task_container<Task, std::remove_reference_t<Parameters>...>>(static_cast<connection_helper<Task>*>(helper.get()), std::move(m_writers), external_callback, std::move(m_construction_parameters));
    }

    std::unique_ptr<abstract_connection_helper> construct_connection_helper() const override
    {
        return std::make_unique<connection_helper<Task>>(m_inputs, m_outputs, std::move(m_listeners));
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

    input_connectors m_inputs;
    output_connectors m_outputs;
    input_writers_ptr m_writers;
    output_callbacks m_listeners;
    std::tuple<std::remove_reference_t<Parameters>...> m_construction_parameters;
};
}
