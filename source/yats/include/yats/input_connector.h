#pragma once

#include <stdexcept>

namespace yats
{

class abstract_output_connector;
template <typename T>
class output_connector;

class abstract_input_connector
{
public:
    explicit abstract_input_connector()
        : m_output(nullptr)
    {
    }

    const abstract_output_connector* output() const
    {
        return m_output;
    }

protected:
    abstract_output_connector* m_output;
};

/// <summary>Allows connecting an output (the source) to an input (the target).</summary>
/// <remarks>Each input may only be connected to one output.</remarks>
template <typename T>
class input_connector : public abstract_input_connector
{
public:
    input_connector() = default;

    input_connector(const input_connector<T>& other) = delete;
    input_connector(input_connector<T>&& other) = delete;

    input_connector<T>& operator=(const input_connector<T>& other) = delete;
    input_connector<T>& operator=(input_connector<T>&& other) = delete;

    /// <summary>Connects output to input.</summary>
    /// <param name="output">Reference to output to connect.</param>
    /// <exception cref="logic_error">Thrown when input is already connected to other output.</exception>
    void operator<<(output_connector<T>& output)
    {
        if (m_output != nullptr)
        {
            throw std::logic_error("Input already connected.");
        }
        m_output = &output;
    }
};
}
