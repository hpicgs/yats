#pragma once
#include <stdexcept>

class OutputProxy;

// Allows the connection of an output to the input.
class InputProxy
{
	friend OutputProxy;
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
	OutputProxy* m_output;
	InputProxy() = default;
private:
};