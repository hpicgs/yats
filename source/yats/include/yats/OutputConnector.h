#pragma once

#include <stdexcept>
#include <tuple>

#include <yats/InputConnector.h>

namespace yats
{

class AbstractOutputConnector
{
public:
    AbstractOutputConnector() = default;

    AbstractOutputConnector& operator>>(AbstractInputConnector& input)
    {
        input << *this;
        return *this;
    }
};

// Allows the connection of an input to the output.
template<typename T>
class OutputConnector : public AbstractOutputConnector
{
public:
    OutputConnector() = default;

    OutputConnector(const OutputConnector<T>& other) = delete;
    OutputConnector(OutputConnector<T>&& other)      = delete;

    OutputConnector<T>& operator=(const OutputConnector<T>& other) = delete;
    OutputConnector<T>& operator=(OutputConnector<T>&& other) = delete;
};

}    // namespace yats
