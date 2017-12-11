#pragma once

namespace yats
{
class AbstractTaskConfigurator;

class AbstractConnector
{
public:
	explicit AbstractConnector(const AbstractTaskConfigurator* const owner)
		: m_owner(owner) {}
	
protected:
	const AbstractTaskConfigurator* const m_owner;
};

}