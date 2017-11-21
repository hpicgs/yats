#pragma once

#include <yatslib/yatslib_api.h>
#include <yatslib/Nodecontainer.h>
#include <yatslib/Output.h>

#include <iostream>
#include <memory>

class AbstractNodeconfigurator
{
public:
	AbstractNodeconfigurator() = default;
	virtual ~AbstractNodeconfigurator() = default;

	virtual std::unique_ptr<AbstractNodecontainer> make() const = 0;

	virtual Input input() = 0;

	virtual Output output() = 0;

private:

};


template <typename Node>
class Nodeconfigurator : public AbstractNodeconfigurator
{
public:
	
	template <typename T, typename S, typename... args>
	void fun(T(S::*)(args...))
	{
		std::cout << typeid(T).name() << std::endl;
	}

	Nodeconfigurator()
	{
		fun(&Node::run);
	}



	std::unique_ptr<AbstractNodecontainer> make() const override
	{
		return std::make_unique<Nodecontainer<Node>>();
	}

	Input input() override
	{
		Input i;
		return i;
	}

	Output output() override
	{
		Output o;
		return o;
	}

private:
};
