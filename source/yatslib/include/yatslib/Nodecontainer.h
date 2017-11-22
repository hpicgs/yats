#pragma once

#include <yatslib/yatslib_api.h>


class AbstractNodecontainer
{
public:
	AbstractNodecontainer() = default;

	virtual ~AbstractNodecontainer() = default;

	virtual bool canRun() const = 0;

private:


};


template <typename T, typename... args>
struct NodeHelper
{
	using RETURN = T;
	using PARAMETER = std::tuple<args...>;
	static constexpr size_t NUM_PARAMETERS = sizeof...(args);
};


template <typename T, typename S, typename... args>
static constexpr NodeHelper<T, args...> Helper(T(S::*)(args...))
{
}


template <typename Node>
class Nodecontainer : public AbstractNodecontainer
{
public:

	using Helper = decltype(Helper(&Node::run));

	Nodecontainer()
		: m_current(0)
	{
	}

	bool canRun() const override
	{
		return m_current == Helper::NUM_PARAMETERS;
	}

private:

	typename Helper::PARAMETER m_parameter;
	Node m_node;
	int m_current;
};
