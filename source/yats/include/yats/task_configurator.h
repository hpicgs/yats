#pragma once

#include <map>
#include <memory>

#include <yats/connection_helper.h>
#include <yats/identifier.h>

namespace yats
{

/**/
class abstract_task_configurator
{
public:
    abstract_task_configurator() = default;
    virtual ~abstract_task_configurator() = default;

    virtual std::unique_ptr<abstract_task_container> make(std::unique_ptr<abstract_connection_helper> helper) const = 0;
    virtual std::unique_ptr<abstract_connection_helper> make2() const = 0;

    static std::vector<std::unique_ptr<abstract_task_container>> build(const std::vector<std::unique_ptr<abstract_task_configurator>>& configurators)
    {
        std::vector<std::unique_ptr<abstract_connection_helper>> helpers;
        for (auto& configurator : configurators)
        {
            helpers.emplace_back(configurator->make2());
        }

        std::map<const abstract_output_connector*, size_t> output_owner;
        for (size_t i = 0; i < configurators.size(); ++i)
        {
            auto outputs = helpers[i]->outputs();
            for (auto output : outputs)
            {
                output_owner.emplace(output.first, i);
            }
        }

        for (auto& helper : helpers)
        {
            auto inputs = helper->inputs();
            for (auto input : inputs)
            {
                auto source_location = input.first->output();
                auto source_task_id = output_owner.at(source_location);

                helpers[source_task_id]->bind(source_location, helper->target(input.first));
            }
        }

        std::vector<std::unique_ptr<abstract_task_container>> tasks;
        for (size_t i = 0; i < configurators.size(); ++i)
        {
            tasks.push_back(configurators[i]->make(std::move(helpers[i])));
        }

        return tasks;
    }
};

template <typename Task, typename... Parameters>
class task_configurator : public abstract_task_configurator
{
public:
    using helper = decltype(make_helper(&Task::run));

    task_configurator(Parameters&&... parameters)
        : m_construction_parameters(std::forward_as_tuple(parameters...))
    {
    }
    
    template<uint64_t Id>
    auto& input()
    {
        constexpr auto index = get_index_by_id_v<Id, typename helper::wrapped_input>;
        using type = typename helper::input_configuration;
        return find<typename helper::wrapped_input, std::tuple_element_t<index, type>>(m_inputs, Id);
    }

    template<uint64_t Id>
    auto& output()
    {
        constexpr auto index = get_index_by_id_v<Id, typename helper::return_base>;
        using type = typename helper::output_configuration;
        return find<typename helper::return_base, std::tuple_element_t<index, type>>(m_outputs, Id);
    }

    std::unique_ptr<abstract_task_container> make(std::unique_ptr<abstract_connection_helper> helper) const override
    {
        auto c = static_cast<connection_helper<Task>*>(helper.get());
        return std::make_unique<task_container<Task, Parameters...>>(c->queue(), c->callbacks(), std::move(m_construction_parameters));
    }

    std::unique_ptr<abstract_connection_helper> make2() const override
    {
        return std::make_unique<connection_helper<Task>>(m_inputs, m_outputs);
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

    typename helper::input_configuration m_inputs;
    typename helper::output_configuration m_outputs;
    const std::tuple<Parameters...> m_construction_parameters;
};
}
