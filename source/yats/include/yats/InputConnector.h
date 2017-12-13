#pragma once

#include <stdexcept>

namespace yats
{

class AbstractOutputConnector;
class AbstractTaskConfigurator;

class AbstractInputConnector
{
public:
	explicit AbstractInputConnector(const AbstractTaskConfigurator* const owner)
		: m_owner(owner), m_output(nullptr) {}

	/// <summary>Connects output to input.</summary>
	/// <param name="output">Reference to output to connect.</param>
	/// <exception cref="logic_error">Thrown when input is already connected to other output.</exception>
	void operator<<(AbstractOutputConnector& output)
	{
		if (m_output != nullptr)
		{
			throw std::logic_error("Input already connected.");
		}
		m_output = &output;
	}
protected:
	const AbstractTaskConfigurator* const m_owner;
	AbstractOutputConnector* m_output;
};

/// <summary>Allows connecting an output (the source) to an input (the target).</summary>
/// <remarks>Each input may only be connected to one output.</remarks>
template <typename T>
class InputConnector : public AbstractInputConnector
{
public:
	explicit InputConnector(const AbstractTaskConfigurator* const owner)
		: AbstractInputConnector(owner) {}
	InputConnector(const InputConnector<T>& other) = delete;
	InputConnector(InputConnector<T>&& other) = default;

	InputConnector<T>& operator=(const InputConnector<T>& other) = delete;
	InputConnector<T>& operator=(InputConnector<T>&& other) = default;
};

}  // namespace yats
