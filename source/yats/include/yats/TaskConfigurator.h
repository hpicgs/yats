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
class ConnectionHelper
{
public:

	using Helper = decltype(MakeHelper(&Task::run));

	ConnectionHelper()
		: m_callbacks(generateCallbacks(m_input, std::make_index_sequence<Helper::ParameterCount>()))
	{
	}

private:

	template <size_t... index>
	static typename Helper::InputCallbacks generateCallbacks(typename Helper::InputQueue &queue, std::integer_sequence<size_t, index...>)
	{
		return std::make_tuple(generateCallback<index>(queue)...);
	}

	template <size_t index>
	static std::tuple_element_t<index, typename Helper::InputCallbacks> generateCallback(typename Helper::InputQueue &queue)
	{
		using ParameterType = std::tuple_element_t<index, typename Helper::InputQueue>::value_type;
		return [&current = std::get<index>(queue)](ParameterType input) mutable
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

	virtual InputConnector& input(const std::string& name) = 0;
	virtual InputConnector& input(uint64_t id) = 0;

	virtual OutputConnector& output(const std::string& name) = 0;
	virtual OutputConnector& output(uint64_t id) = 0;
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
		return std::make_unique<TaskContainer<Task>>();
	}

	InputConnector& input(const std::string& name) override
	{
		return m_inputs.at(id(name.c_str()));
	}

	InputConnector& input(uint64_t id) override
	{
		return m_inputs.at(id);
	}

	OutputConnector& output(const std::string& name) override
	{
		return m_outputs.at(id(name.c_str()));
	}

	OutputConnector& output(uint64_t id) override
	{
		return m_outputs.at(id);
	}

	static void build(std::map<std::string, std::unique_ptr<AbstractTaskConfigurator>> &configurators)
	{
		std::vector<AbstractTaskConfigurator*> confs;
		for (auto &c : configurators)
		{
			confs.push_back(c.second.get());
		}

		// Instantiate the InputQueues
		// Instantiate the ReturnCallbacks
		// Instantiate connection functions.

		// For each InputQueue instantiate the set functions
		// Lookup the OutputConnector from the InputConnector
		// Throw if a InputConnector has a nullptr OutputConnector
		// Add the function to the correct ReturnCallback list that is assosiated with the OutputConnector

		// Construct all TaskContainer
	}

protected:

	std::unique_ptr<AbstractConnectionHelper> make()
	{
		return std::make_unique<ConnectionHelper<Task>>();
	}

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
		m_inputs.insert(std::make_pair(currentInput::ID, InputConnector(this)));
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
		m_outputs.insert(std::make_pair(currentOutput::ID, OutputConnector(this)));
		parseOutputParameter<Index + 1, Max>();
	}

	std::map<uint64_t, InputConnector> m_inputs;
	std::map<uint64_t, OutputConnector> m_outputs;
};

}  // namespace yats
