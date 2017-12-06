
#pragma once

#include <stdexcept>


class OutputConnector;

/// <summary>Allows connecting an output (the source) to an input (the target).</summary>
/// <remarks>Each input may only be connected to one output.</remarks>
class InputConnector
{
public:
	InputConnector() : m_output(nullptr) {}
	InputConnector(const InputConnector& other) = delete;
	InputConnector(InputConnector&& other) = default;

	/// <summary>Connects output to input.</summary>
	/// <param name="output">Reference to output to connect.</param>
	/// <exception cref="logic_error">Thrown when input is already connected to other output.</exception>
	void operator<<(OutputConnector& output)
	{
		if (m_output != nullptr)
		{
			throw std::logic_error("Input already connected.");
		}
		m_output = &output;
	}

	InputConnector& operator=(const InputConnector& other) = delete;
	InputConnector& operator=(InputConnector&& other) = default;

protected:
	OutputConnector* m_output;
};
