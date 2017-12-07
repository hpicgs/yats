#pragma once

#include <stdexcept>

namespace yats
{

class AbstractTaskConfigurator;
class OutputConnector;

/// <summary>Allows connecting an output (the source) to an input (the target).</summary>
/// <remarks>Each input may only be connected to one output.</remarks>
class InputConnector
{
public:
	explicit InputConnector(const AbstractTaskConfigurator* const owner) : m_owner(owner), m_output(nullptr) {}
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
	const AbstractTaskConfigurator* const m_owner;
	OutputConnector* m_output;
};

}  // namespace yats
