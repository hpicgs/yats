#pragma once
#include <stdexcept>
#include <yats/InputProxy.h>

// Allows the connection of an Output to the input.
class OutputProxy
{
public:
	OutputProxy(const OutputProxy& output) = delete;

	OutputProxy& operator>>(InputProxy& input)
	{
		if (input.m_output != nullptr)
		{
			throw std::logic_error("Input already connected.");
		}
		input.m_output = this;
		return *this;
	}
protected:
	OutputProxy() = default;
private:
};