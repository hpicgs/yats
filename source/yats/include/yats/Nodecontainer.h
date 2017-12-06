#pragma once

#include <tuple>

class AbstractNodecontainer
{
public:
	AbstractNodecontainer() = default;

	virtual ~AbstractNodecontainer() = default;

	virtual bool canRun() const = 0;

private:


};


template <typename Return, typename... ParameterTypes>
struct NodeHelper
{
	template <typename CompoundType>
	static typename CompoundType::value_type transform();

	using WrappedInput = std::tuple<ParameterTypes...>;
	using Input = std::tuple<decltype(transform<ParameterTypes>())...>;
	using ReturnType = Return;

	static constexpr size_t ParameterCount = sizeof...(ParameterTypes);
};


template <typename ReturnType, typename TaskType, typename... ParameterTypes>
static constexpr NodeHelper<ReturnType, ParameterTypes...> MakeHelper(ReturnType(TaskType::*)(ParameterTypes...))
{
}


template <typename Node>
class Nodecontainer : public AbstractNodecontainer
{
public:

	using Helper = decltype(MakeHelper(&Node::run));

	Nodecontainer()
		: m_current(0)
	{
	}

	bool canRun() const override
	{
		return m_current == Helper::ParameterCount;
	}

private:

	typename Helper::Input m_parameter;
	Node m_node;
	int m_current;
};
