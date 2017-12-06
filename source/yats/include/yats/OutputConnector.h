
#pragma once

#include <stdexcept>
#include <tuple>

#include "InputConnector.h"


template<typename... Args>
using OutputBundle = std::tuple<Args...>;

// Allows the connection of an input to the output.
class OutputConnector
{
public:
	OutputConnector() = default;
	OutputConnector(const OutputConnector& other) = delete;
	OutputConnector(OutputConnector&& other) = default;

	OutputConnector& operator>>(InputConnector& input)
	{
		input << *this;
		return *this;
	}

	OutputConnector& operator=(const OutputConnector& other) = delete;
	OutputConnector& operator=(OutputConnector&& other) = default;
};
