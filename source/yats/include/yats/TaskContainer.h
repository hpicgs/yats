#pragma once

#include <tuple>
#include <utility>

#include <yats/Util.h>

namespace yats
{

class AbstractTaskContainer
{
public:
	AbstractTaskContainer() = default;

	virtual ~AbstractTaskContainer() = default;

	virtual void run() = 0;

private:


};


template <typename Task>
class TaskContainer : public AbstractTaskContainer
{
public:

	using Helper = decltype(MakeHelper(&Task::run));

	TaskContainer() = default;

	void run() override
	{
		call();
	}

private:

	template <typename T = Helper::ReturnType>
	std::enable_if_t<!std::is_same<T, void>::value> call()
	{
		auto values = invoke(std::make_index_sequence<Helper::ParameterCount>());

		// write values
	}

	template <typename T = Helper::ReturnType>
	std::enable_if_t<std::is_same<T, void>::value> call()
	{
		invoke(std::make_index_sequence<Helper::ParameterCount>());
	}

	template <size_t... index, typename T = Helper::ReturnType, typename SFINAE = std::enable_if_t<!std::is_same<T, void>::value>>
	auto invoke(std::integer_sequence<size_t, index...>)
	{
		return m_task.run(get<index>()...);
	}

	template <size_t... index, typename T = Helper::ReturnType, typename SFINAE = std::enable_if_t<std::is_same<T, void>::value>>
	SFINAE invoke(std::integer_sequence<size_t, index...>)
	{
		m_task.run(get<index>()...);
	}

	template <size_t index>
	auto get()
	{
		auto elem = std::get<index>(m_input);
		auto value = elem.front();
		elem.pop();

		return value;
	}

	typename Helper::InputQueue m_input;
	typename Helper::ReturnCallbacks m_output;
	Task m_task;
};

} // namespace yats
