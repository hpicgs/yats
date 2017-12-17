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
};

template <typename Task>
class ConnectionHelper : public AbstractConnectionHelper
{
public:

	using Helper = decltype(MakeHelper(&Task::run));

	ConnectionHelper(const std::map<uint64_t, InputConnector> &, const std::map<uint64_t, OutputConnector> &)
		: m_input(std::make_unique<typename Helper::InputQueueBase>())
		, m_callbacks(generateCallbacks(m_input, std::make_index_sequence<Helper::ParameterCount>()))
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

	virtual AbstractInputConnector& input(const std::string& name) = 0;
	virtual AbstractInputConnector& input(uint64_t id) = 0;

	virtual AbstractOutputConnector& output(const std::string& name) = 0;
	virtual AbstractOutputConnector& output(uint64_t id) = 0;

	virtual std::unique_ptr<AbstractConnectionHelper> make2() const = 0;
};


template <typename Task>
class TaskConfigurator : public AbstractTaskConfigurator
{
public:

	using Helper = decltype(MakeHelper(&Task::run));

	TaskConfigurator()
	{
		parseInputParameters();
		parseOutputParameters();
	}

	std::unique_ptr<AbstractTaskContainer> make() const override
	{
		return nullptr;
		//return std::make_unique<TaskContainer<Task>>();
	}

	AbstractInputConnector& input(const std::string& name) override
	{
		return *(m_inputs.at(id(name.c_str())));
	}

	AbstractInputConnector& input(uint64_t id) override
	{
		return *(m_inputs.at(id));
	}

	AbstractOutputConnector& output(const std::string& name) override
	{
		return *(m_outputs.at(id(name.c_str())));
	}

	AbstractOutputConnector& output(uint64_t id) override
	{
		return *(m_outputs.at(id).get());
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

		/*
		std::map<OutputConnector*, size_t> outputOwner;
		for (size_t i = 0; i < confs.size(); ++i)
		{
			auto outputs = confs[i]->outputs();
			for (auto output : outputs)
			{
				outputOwner.emplace(output, i);
			}
		}

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

	std::unique_ptr<AbstractConnectionHelper> make2() const override
	{
		return std::make_unique<ConnectionHelper<Task>>(m_inputs, m_outputs);
	}

protected:

	void parseInputParameters()
	{
		parseInputParameter<0>();
	}

	template<size_t Index>
	std::enable_if_t<Index == Helper::ParameterCount> parseInputParameter()
	{

	}

	template<size_t Index>
	std::enable_if_t<Index < Helper::ParameterCount> parseInputParameter()
	{
		using currentInput = std::tuple_element_t<Index, typename Helper::WrappedInput>;
		m_inputs.emplace(currentInput::ID, std::make_unique<InputConnector<currentInput>>(this));
		parseInputParameter<Index + 1>();
	}

	template<typename T = typename Helper::ReturnType>
	std::enable_if_t<std::is_same<T, void>::value> parseOutputParameters()
	{

	}

	template<typename T = typename Helper::ReturnType>
	std::enable_if_t<!std::is_same<T, void>::value> parseOutputParameters()
	{
		parseOutputParameter<0, std::tuple_size<typename Helper::ReturnType>::value>();
	}

	template<size_t Index, size_t Max>
	std::enable_if_t<Index == Max> parseOutputParameter()
	{

	}

	template<size_t Index, size_t Max>
	std::enable_if_t<Index < Max> parseOutputParameter()
	{
		using currentOutput = std::tuple_element_t<Index, typename Helper::ReturnType>;
		m_outputs.emplace(currentOutput::ID, std::make_unique<OutputConnector<currentOutput>>(this));
		parseOutputParameter<Index + 1, Max>();
	}

	std::map<uint64_t, std::unique_ptr<AbstractInputConnector>> m_inputs;
	std::map<uint64_t, std::unique_ptr<AbstractOutputConnector>> m_outputs;
};

}  // namespace yats
