
#pragma once

#include <stdexcept>

#include <yats/InputProxy.h>


// Allows the connection of an input to the output.
class OutputProxy
{
public:
	OutputProxy(const OutputProxy& output) = delete;

	OutputProxy& operator>>(InputProxy& input)
	{
		input << *this;
		return *this;
	}

protected:
	OutputProxy() = default;
};
