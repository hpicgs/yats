#pragma once

#include <yats/input_connector.h>

namespace yats
{

/**
 * Represents an output which can be connected to an input.
 */
class abstract_output_connector
{
public:
    abstract_output_connector() = default;

    virtual ~abstract_output_connector() = default;

    abstract_output_connector(const abstract_output_connector& other) = delete;
    abstract_output_connector(abstract_output_connector&& other) = delete;

    abstract_output_connector& operator=(const abstract_output_connector& other) = delete;
    abstract_output_connector& operator=(abstract_output_connector&& other) = delete;
};

/**
 * Represents an output (the source) which can be connected to an input (the target).
 * Note: An output may be connected to more than one input.
 * @param T Type of output value
 */
template <typename T>
class output_connector : public abstract_output_connector
{
public:
    output_connector() = default;
    ~output_connector() = default;

    output_connector(const output_connector<T>& other) = delete;
    output_connector(output_connector<T>&& other) = delete;

    output_connector<T>& operator=(const output_connector<T>& other) = delete;
    output_connector<T>& operator=(output_connector<T>&& other) = delete;

    /**
     * Connects the output {@code input}.
     * @param input Reference to input to connect to.
     * @throws logic_error Thrown when {@code input} is already connected to another output.
     */
    output_connector<T>& operator>>(input_connector<T>& input)
    {
        input << *this;
        return *this;
    }
};
}
