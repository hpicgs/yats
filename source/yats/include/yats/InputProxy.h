
#pragma once

#include <stdexcept>


class OutputProxy;

/// <summary>Allows connecting an output (the source) to an input (the target).</summary>
/// <remarks>Each input may only be connected to one output.</remarks>
class InputProxy
{
public:
	InputProxy() : m_output(nullptr) {}
	InputProxy(const InputProxy& input) = delete;

	/// <summary>Connects output to input.</summary>
	/// <param name="output">Reference to output to connect.</param>
	/// <exception cref="logic_error">Thrown when input is already connected to other output.</exception>
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
};
