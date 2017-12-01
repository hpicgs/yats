
#pragma once

#include <stdexcept>
#include <tuple>

#include "InputProxy.h"


template<typename... Args>
using OutputBundle = std::tuple<Args...>;

// Allows the connection of an input to the output.
class OutputProxy
{
public:
	OutputProxy() = default;
	OutputProxy(const OutputProxy& output) = delete;

	OutputProxy& operator>>(InputProxy& input)
	{
		input << *this;
		return *this;
	}
};
