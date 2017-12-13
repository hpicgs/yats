#pragma once

#include <stdexcept>
#include <tuple>

#include <yats/AbstractConnector.h>
#include <yats/InputConnector.h>

namespace yats
{

class AbstractOutputConnector
{
public:
	explicit AbstractOutputConnector(const AbstractTaskConfigurator* const owner)
		: m_owner(owner) {}

	AbstractOutputConnector& operator>>(AbstractInputConnector& input)
	{
		input << *this;
		return *this;
	}
protected:
	const AbstractTaskConfigurator* const m_owner;
};


// Allows the connection of an input to the output.
template <typename T>
class OutputConnector : public AbstractOutputConnector
{
public:
	explicit OutputConnector(const AbstractTaskConfigurator* const owner)
		: AbstractOutputConnector(owner) {}
	OutputConnector(const OutputConnector<T>& other) = delete;
	OutputConnector(OutputConnector<T>&& other) = default;

	OutputConnector<T>& operator=(const OutputConnector<T>& other) = delete;
	OutputConnector<T>& operator=(OutputConnector<T>&& other) = default;
};

}  // namespace yats
