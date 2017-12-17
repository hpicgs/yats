#pragma once

#include <memory>
#include <map>

#include <yats/Identifier.h>
#include <yats/InputConnector.h>
#include <yats/TaskContainer.h>
#include <yats/OutputConnector.h>

namespace yats
{

class AbstractConnectionHelper
{
public:

	template <typename Type>
	using Locations = std::map<const Type*, size_t>;

	AbstractConnectionHelper(Locations<AbstractInputConnector> input, Locations<AbstractOutputConnector> output)
		: m_input(input)
		, m_output(output)
	{
	}

	virtual void bind() = 0;
	
	const auto& inputs()
	{
		return m_input;
	}

	const auto& outputs()
	{
		return m_output;
	}

protected:

	const Locations<AbstractInputConnector> m_input;
	const Locations<AbstractOutputConnector> m_output;
};

template <typename Task>
class ConnectionHelper : public AbstractConnectionHelper
{
public:

	using Helper = decltype(MakeHelper(&Task::run));
	using InputSequence = std::make_index_sequence<std::tuple_size<typename Helper::InputConfiguration>::value>;
	using OutputSequence = std::make_index_sequence<std::tuple_size<typename Helper::OutputConfiguration>::value>;

	ConnectionHelper(const typename Helper::InputConfiguration &inputs, const typename Helper::OutputConfiguration &outputs)
		: AbstractConnectionHelper(map<AbstractInputConnector>(inputs, InputSequence()), map<AbstractOutputConnector>(outputs, OutputSequence()))
		, m_input(std::make_unique<typename Helper::InputQueueBase>())
		, m_callbacks(generateCallbacks(m_input, std::make_index_sequence<Helper::ParameterCount>()))
	{
	}

	void bind() override
	{

	}

private:

	template <size_t... index>
	static typename Helper::InputCallbacks generateCallbacks(typename Helper::InputQueue &queue, std::integer_sequence<size_t, index...>)
	{
		// Prevent a warning about unused parameter when handling a run function with no parameters.
		(void) queue;
		return std::make_tuple(generateCallback<index>(queue)...);
	}

	template <size_t index>
	static typename std::tuple_element_t<index, typename Helper::InputCallbacks> generateCallback(typename Helper::InputQueue &queue)
	{
		using ParameterType = typename std::tuple_element_t<index, typename Helper::InputQueueBase>::value_type;
		return [&current = std::get<index>(*queue)](ParameterType input) mutable
		{
			current.push(input);
		};
	}

	template <typename LocationType, typename SequenceType, size_t... index>
	static Locations<LocationType> map(const SequenceType &outputs, std::index_sequence<index...>)
	{
		// Prevent a warning about unused parameter when handling a run function with no parameters.
		(void) outputs;
		return { std::make_pair(&std::get<index>(outputs), index)... };
	}

	typename Helper::InputQueue m_input;
	typename Helper::ReturnCallbacks m_output;
	typename Helper::InputCallbacks m_callbacks;
};

/**/
class AbstractTaskConfigurator
{
public:
	AbstractTaskConfigurator() = default;
	virtual ~AbstractTaskConfigurator() = default;

	virtual std::unique_ptr<AbstractTaskContainer> make() const = 0;
	virtual std::unique_ptr<AbstractConnectionHelper> make2() const = 0;

	virtual AbstractInputConnector& input(const std::string& name) = 0;
	virtual AbstractInputConnector& input(uint64_t id) = 0;

	virtual AbstractOutputConnector& output(const std::string& name) = 0;
	virtual AbstractOutputConnector& output(uint64_t id) = 0;
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

	static void build(std::map<std::string, std::unique_ptr<AbstractTaskConfigurator>> &configurators)
	{
		std::vector<AbstractTaskConfigurator*> confs;
		for (auto &c : configurators)
		{
			confs.push_back(c.second.get());
		}

		std::vector<std::unique_ptr<AbstractConnectionHelper>> helpers;
		for (auto c : confs)
		{
			helpers.emplace_back(c->make2());
		}

		
		std::map<const AbstractOutputConnector*, size_t> outputOwner;
		for (size_t i = 0; i < confs.size(); ++i)
		{
			auto outputs = helpers[i]->outputs();
			for (auto output : outputs)
			{
				outputOwner.emplace(output.first, i);
			}
		}

		/*
		for (auto c : confs)
		{
			auto inputs = c->inputs();
			for (auto input : inputs)
			{
				auto id = outputOwner[input->source()];
			}
		}
		*/

		// Throw if a InputConnector has a nullptr OutputConnector
		// Add the function to the correct ReturnCallback list that is assosiated with the OutputConnector

		// Construct all TaskContainer
	}

	std::unique_ptr<AbstractTaskContainer> make() const override
	{
		return nullptr;
		//return std::make_unique<TaskContainer<Task>>();
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
