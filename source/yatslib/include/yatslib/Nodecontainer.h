#pragma once

#include <yatslib/yatslib_api.h>


class AbstractNodecontainer
{
public:
	AbstractNodecontainer() = default;
	virtual ~AbstractNodecontainer() = default;

private:


};


template <typename Node>
class Nodecontainer : public AbstractNodecontainer
{
public:
	Nodecontainer() = default;


private:

	Node m_node;
};
