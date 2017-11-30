
#pragma once

#include <stdexcept>


class OutputProxy;

// Allows the connection of an output to the input.
class InputProxy
{
public:
	InputProxy(const InputProxy& input) = delete;

	void operator<<(OutputProxy& output)
	{
		if (m_output != nullptr)
		{
			throw std::logic_error("Input already connected.");
		}
		m_output = &output;
	}

protected:
	InputProxy() = default;

	OutputProxy* m_output;
};
