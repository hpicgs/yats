#pragma once

#include <stdexcept>

namespace yats
{

class AbstractOutputConnector;
class AbstractTaskConfigurator;

class AbstractInputConnector
{
public:
    explicit AbstractInputConnector()
        : m_output(nullptr)
    {
    }

    const AbstractOutputConnector* output() const
    {
        return m_output;
    }

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
    AbstractOutputConnector* m_output;
};

/// <summary>Allows connecting an output (the source) to an input (the target).</summary>
/// <remarks>Each input may only be connected to one output.</remarks>
template<typename T>
class InputConnector : public AbstractInputConnector
{
public:
    InputConnector() = default;

    InputConnector(const InputConnector<T>& other) = delete;
    InputConnector(InputConnector<T>&& other)      = delete;

    InputConnector<T>& operator=(const InputConnector<T>& other) = delete;
    InputConnector<T>& operator=(InputConnector<T>&& other) = delete;
};

}    // namespace yats
