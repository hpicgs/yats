#pragma once

#include <iostream>
#include <memory>
#include <vector>

#include <yats/InputProxy.h>
#include <yats/Nodecontainer.h>
#include <yats/OutputProxy.h>


/**/
class AbstractTaskConfigurator
{
public:
	AbstractTaskConfigurator() = default;
	virtual ~AbstractTaskConfigurator() = default;

	virtual std::unique_ptr<AbstractNodecontainer> make() const = 0;

	virtual InputProxy input(const std::string& name) = 0;
	virtual InputProxy input(size_t id) = 0;

	virtual OutputProxy output(const std::string& name) = 0;
	virtual OutputProxy output(size_t id) = 0;

private:

};


template <typename Node>
class Nodeconfigurator : public AbstractNodeconfigurator
{
public:
	
	using Helper = decltype(MakeHelper(&Node::run));

	Nodeconfigurator() = default;

	std::unique_ptr<AbstractNodecontainer> make() const override
	{
		return std::make_unique<Nodecontainer<Node>>();
	}

	Input input() override
	{
		return i[0];
	}

	Output output() override
	{
		return o[0];
	}

private:

	std::vector<Input> i;
	std::vector<Output> o;
};
