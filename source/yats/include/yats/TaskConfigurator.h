#pragma once

#include <memory>
#include <map>

#include <yats/Identifier.h>
#include <yats/InputConnector.h>
#include <yats/TaskContainer.h>
#include <yats/OutputConnector.h>

namespace yats
{

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

	AbstractInputConnector& input(const std::string& name) override
	{
		return m_inputs.at(id(name.c_str()));
	}

	AbstractInputConnector& input(uint64_t id) override
	{
		return m_inputs.at(id);
	}

	AbstractOutputConnector& output(const std::string& name) override
	{
		return m_outputs.at(id(name.c_str()));
	}

	AbstractOutputConnector& output(uint64_t id) override
	{
		return m_outputs.at(id);
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
		m_inputs.insert(std::make_pair(currentInput::ID, InputConnector<currentInput>(this)));
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
		m_outputs.insert(std::make_pair(currentOutput::ID, OutputConnector<currentOutput>(this)));
		parseOutputParameter<Index + 1, Max>();
	}

	std::map<uint64_t, AbstractInputConnector> m_inputs;
	std::map<uint64_t, AbstractOutputConnector> m_outputs;
};

}  // namespace yats
