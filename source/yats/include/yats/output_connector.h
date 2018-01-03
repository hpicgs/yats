#pragma once

#include <stdexcept>
#include <tuple>

#include <yats/input_connector.h>

namespace yats
{

class abstract_output_connector
{
public:
    abstract_output_connector() = default;

    abstract_output_connector& operator>>(abstract_input_connector& input)
    {
        input << *this;
        return *this;
    }
};

// Allows the connection of an input to the output.
template <typename T>
class output_connector : public abstract_output_connector
{
public:
    output_connector() = default;

    output_connector(const output_connector<T>& other) = delete;
    output_connector(output_connector<T>&& other) = delete;

    output_connector<T>& operator=(const output_connector<T>& other) = delete;
    output_connector<T>& operator=(output_connector<T>&& other) = delete;
};
}
