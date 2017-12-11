#pragma once

#include <stdexcept>
#include <tuple>

#include <yats/AbstractConnector.h>
#include <yats/InputConnector.h>

namespace yats
{

// Allows the connection of an input to the output.
class OutputConnector : public AbstractConnector
{
public:
	explicit OutputConnector(const AbstractTaskConfigurator* const owner) : AbstractConnector(owner) {}
	OutputConnector(const OutputConnector& other) = delete;
	OutputConnector(OutputConnector&& other) = default;

	OutputConnector& operator>>(InputConnector& input)
	{
		input << *this;
		return *this;
	}

	OutputConnector& operator=(const OutputConnector& other) = delete;
	OutputConnector& operator=(OutputConnector&& other) = default;

protected:
};

}  // namespace yats
