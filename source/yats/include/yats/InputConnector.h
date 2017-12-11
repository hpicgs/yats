#pragma once

#include <stdexcept>
#include <yats/AbstractConnector.h>

namespace yats
{

template <typename T>
class OutputConnector<T>;

class AbstractInputConnector : public AbstractConnector
{
public:
	explicit AbstractInputConnector(const AbstractTaskConfigurator* const owner)
		: AbstractConnector(owner) {}
};

/// <summary>Allows connecting an output (the source) to an input (the target).</summary>
/// <remarks>Each input may only be connected to one output.</remarks>
template <typename T>
class InputConnector : public AbstractConnector
{
public:
	explicit InputConnector(const AbstractTaskConfigurator* const owner)
		: AbstractInputConnector(owner),  m_output(nullptr) {}
	InputConnector(const InputConnector<T>& other) = delete;
	InputConnector(InputConnector<T>&& other) = default;

	/// <summary>Connects output to input.</summary>
	/// <param name="output">Reference to output to connect.</param>
	/// <exception cref="logic_error">Thrown when input is already connected to other output.</exception>
	void operator<<(OutputConnector<T>& output)
	{
		if (m_output != nullptr)
		{
			throw std::logic_error("Input already connected.");
		}
		m_output = &output;
	}

	InputConnector<T>& operator=(const InputConnector<T>& other) = delete;
	InputConnector<T>& operator=(InputConnector<T>&& other) = default;

protected:
	OutputConnector<T>* m_output;
};

}  // namespace yats
