#pragma once

#include <stdexcept>
#include <tuple>

#include <yats/AbstractConnector.h>
#include <yats/InputConnector.h>

namespace yats
{

class AbstractOutputConnector : public AbstractConnector
{
public:
	explicit AbstractOutputConnector(const AbstractTaskConfigurator* const owner)
		: AbstractConnector(owner) {}
};


// Allows the connection of an input to the output.
template <typename T>
class OutputConnector : public AbstractConnector
{
public:
	explicit OutputConnector(const AbstractTaskConfigurator* const owner)
		: AbstractOutputConnector(owner) {}
	OutputConnector(const OutputConnector<T>& other) = delete;
	OutputConnector(OutputConnector<T>&& other) = default;

	OutputConnector<T>& operator>>(InputConnector<T>& input)
	{
		input << *this;
		return *this;
	}

	OutputConnector<T>& operator=(const OutputConnector<T>& other) = delete;
	OutputConnector<T>& operator=(OutputConnector<T>&& other) = default;

protected:
};

}  // namespace yats
