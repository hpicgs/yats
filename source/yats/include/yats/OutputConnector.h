#pragma once

#include <stdexcept>
#include <tuple>

#include <yats/InputConnector.h>

namespace yats
{

class AbstractTaskConfigurator;

// Allows the connection of an input to the output.
class OutputConnector
{
public:
	explicit OutputConnector(const AbstractTaskConfigurator* const owner) : m_owner(owner) {}
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
	const AbstractTaskConfigurator* const m_owner;
};

}  // namespace yats
