
#pragma once

#include <memory>
#include <map>

#include "InputProxy.h"
#include "Nodecontainer.h"
#include "OutputProxy.h"


/**/
class AbstractTaskConfigurator
{
public:
	AbstractTaskConfigurator() = default;
	virtual ~AbstractTaskConfigurator() = default;

	virtual std::unique_ptr<AbstractNodecontainer> make() const = 0;

	virtual InputProxy& input(const std::string& name) = 0;
	virtual InputProxy& input(size_t id) = 0;

	virtual OutputProxy& output(const std::string& name) = 0;
	virtual OutputProxy& output(size_t id) = 0;
};


template <typename Node>
class TaskConfigurator : public AbstractTaskConfigurator
{
public:

	using Helper = decltype(MakeHelper(&Node::run));

	TaskConfigurator()
	{
		parseInputParameters();
		parseOutputParameters();
	}

	std::unique_ptr<AbstractNodecontainer> make() const override
	{
		return std::make_unique<Nodecontainer<Node>>();
	}

	InputProxy& input(const std::string& name) override
	{
		//TODO convert name to id
		return InputProxy();
	}

	InputProxy& input(size_t id) override
	{
		return m_inputs[id];
	}

	OutputProxy& output(const std::string& name) override
	{
		//TODO convert name to id
		return OutputProxy();
	}

	OutputProxy& output(size_t id) override
	{
		return m_outputs[id];
	}

protected:
	void parseInputParameters()
	{
		parseInputParameter<0>();
	}

	template<size_t Index>
	std::enable_if_t<Index == Helper::NUM_PARAMETERS> parseInputParameter()
	{

	}

	template<size_t Index>
	std::enable_if_t<Index < Helper::NUM_PARAMETERS> parseInputParameter()
	{
		using currentInput = std::tuple_element_t<Index, Helper::PARAMETER>;
		m_inputs[currentInput::ID] = InputProxy();
		parseInputParameter<Index + 1>();
	}

	template<typename T = typename Helper::RETURN>
	std::enable_if_t<std::is_same_v<T, void>> parseOutputParameters()
	{

	}

	template<typename T = typename Helper::RETURN>
	std::enable_if_t<!std::is_same_v<T, void>> parseOutputParameters()
	{
		parseOutputParameter<0, std::tuple_size_v<typename Helper::RETURN>>();
	}

	template<size_t Index, size_t Max>
	std::enable_if_t<Index == Max> parseOutputParameter()
	{

	}

	template<size_t Index, size_t Max>
	std::enable_if_t<Index < Max> parseOutputParameter()
	{
		using currentOutput = std::tuple_element_t<Index, Helper::RETURN>;
		m_outputs[currentOutput::ID] = OutputProxy();
		parseOutputParameter<Index + 1, Max>();
	}

	std::map<size_t, InputProxy> m_inputs;
	std::map<size_t, OutputProxy> m_outputs;
};
