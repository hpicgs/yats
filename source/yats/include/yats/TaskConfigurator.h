#pragma once

#include <memory>
#include <map>

#include <yats/ConnectionHelper.h>
#include <yats/Identifier.h>

namespace yats
{

/**/
class AbstractTaskConfigurator
{
public:
	AbstractTaskConfigurator() = default;
	virtual ~AbstractTaskConfigurator() = default;

	virtual std::unique_ptr<AbstractTaskContainer> make(std::unique_ptr<AbstractConnectionHelper> helper) const = 0;
	virtual std::unique_ptr<AbstractConnectionHelper> make2() const = 0;

	virtual AbstractInputConnector& input(const std::string& name) = 0;
	virtual AbstractInputConnector& input(uint64_t id) = 0;

	virtual AbstractOutputConnector& output(const std::string& name) = 0;
	virtual AbstractOutputConnector& output(uint64_t id) = 0;

	static std::vector<std::unique_ptr<AbstractTaskContainer>> build(const std::map<std::string, std::unique_ptr<AbstractTaskConfigurator>> &namedConfigurators)
	{
		std::vector<AbstractTaskConfigurator*> configurators;
		for (auto &configurator : namedConfigurators)
		{
			configurators.push_back(configurator.second.get());
		}

		std::vector<std::unique_ptr<AbstractConnectionHelper>> helpers;
		for (auto configurator : configurators)
		{
			helpers.emplace_back(configurator->make2());
		}


		std::map<const AbstractOutputConnector*, size_t> outputOwner;
		for (size_t i = 0; i < configurators.size(); ++i)
		{
			auto outputs = helpers[i]->outputs();
			for (auto output : outputs)
			{
				outputOwner.emplace(output.first, i);
			}
		}

		for (auto &helper : helpers)
		{
			auto inputs = helper->inputs();
			for (auto input : inputs)
			{
				auto sourceLocation = input.first->output();
				auto sourceTaskId = outputOwner.at(sourceLocation);

				helpers[sourceTaskId]->bind(sourceLocation, helper->target(input.first));
			}
		}

		std::vector<std::unique_ptr<AbstractTaskContainer>> tasks;
		for (size_t i = 0; i < configurators.size(); ++i)
		{
			tasks.emplace_back(configurators[i]->make(std::move(helpers[i])));
		}

		return tasks;
	}
};


template <typename Task>
class TaskConfigurator : public AbstractTaskConfigurator
{
public:

	using Helper = decltype(MakeHelper(&Task::run));

	TaskConfigurator() = default;

	AbstractInputConnector& input(const std::string& name) override
	{
		return find<typename Helper::WrappedInput, AbstractInputConnector>(m_inputs, id(name.c_str()));
	}

	AbstractInputConnector& input(uint64_t id) override
	{
		return find<typename Helper::WrappedInput, AbstractInputConnector>(m_inputs, id);
	}

	AbstractOutputConnector& output(const std::string& name) override
	{
		return find<typename Helper::ReturnBase, AbstractOutputConnector>(m_outputs, id(name.c_str()));
	}

	AbstractOutputConnector& output(uint64_t id) override
	{
		return find<typename Helper::ReturnBase, AbstractOutputConnector>(m_outputs, id);
	}

	std::unique_ptr<AbstractTaskContainer> make(std::unique_ptr<AbstractConnectionHelper> helper) const override
	{
		auto c = static_cast<ConnectionHelper<Task>*>(helper.get());
		return std::make_unique<TaskContainer<Task>>(c->queue(), c->callbacks());
	}

	std::unique_ptr<AbstractConnectionHelper> make2() const override
	{
		return std::make_unique<ConnectionHelper<Task>>(m_inputs, m_outputs);
	}

protected:

	template <typename IdTuple, typename Return, typename Parameter>
	Return& find(Parameter &tuple, uint64_t id)
	{
		auto connector = get<IdTuple, Return>(tuple, id);
		if (connector)
		{
			return *connector;
		}
		throw std::runtime_error("Id not found.");
	}

	template <typename IdTuple, typename Return, size_t Index = 0, typename Parameter = int>
	std::enable_if_t<Index < std::tuple_size<IdTuple>::value, Return*> get(Parameter &tuple, uint64_t id)
	{
		auto elem = &std::get<Index>(tuple);
		if (id == std::tuple_element_t<Index, IdTuple>::ID)
		{
			return elem;
		}
		return get<IdTuple, Return, Index + 1>(tuple, id);
	}

	template <typename IdTuple, typename Return, size_t Index = 0, typename Parameter = int>
	std::enable_if_t<Index == std::tuple_size<IdTuple>::value, Return*> get(Parameter &, uint64_t)
	{
		return nullptr;
	}

	typename Helper::InputConfiguration m_inputs;
	typename Helper::OutputConfiguration m_outputs;
};

}  // namespace yats
