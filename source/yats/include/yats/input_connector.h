#pragma once

#include <stdexcept>

namespace yats
{

class abstract_output_connector;
template <typename T>
class output_connector;

/**
 * Represents an input to which an output can be connected.
 */
class abstract_input_connector
{
public:
    explicit abstract_input_connector()
        : m_output(nullptr)
    {
    }

    virtual ~abstract_input_connector() = default;

    abstract_input_connector(const abstract_input_connector& other) = delete;
    abstract_input_connector(abstract_input_connector&& other) = delete;

    abstract_input_connector& operator=(const abstract_input_connector& other) = delete;
    abstract_input_connector& operator=(abstract_input_connector&& other) = delete;

    /**
     * Constant pointer to the output the input is connected to.
     * @returns A constant pointer to the output the input is connected to or nullptr if
     * the input is not connected to an output.
     */
    const abstract_output_connector* output() const
    {
        return m_output;
    }

protected:
    abstract_output_connector* m_output;
};

/**
 * Represents an input (the target) to which an output (the source) can be connected.
 * <p>Note: Each input may only be connected to one output.</p>
 * <p>T: Type of input value</p>
 */
template <typename T>
class input_connector : public abstract_input_connector
{
public:
    input_connector() = default;
    ~input_connector() = default;

    input_connector(const input_connector<T>& other) = delete;
    input_connector(input_connector<T>&& other) = delete;

    input_connector<T>& operator=(const input_connector<T>& other) = delete;
    input_connector<T>& operator=(input_connector<T>&& other) = delete;

    /**
     * Connects output to input.
     * @param output Reference to output to connect.
     * @throws logic_error Thrown when input is already connected to another output.
     */
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
