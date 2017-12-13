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
		invoke(std::make_index_sequence<Helper::ParameterCount>());
	}

private:

	template <size_t... index, typename T = typename Helper::ReturnType>
	std::enable_if_t<!std::is_same<T, void>::value> invoke(std::integer_sequence<size_t, index...>)
	{
		auto output = m_task.run(get<index>()...);
		write(output);
	}

	template <size_t... index, typename T = typename Helper::ReturnType>
	std::enable_if_t<std::is_same<T, void>::value> invoke(std::integer_sequence<size_t, index...>)
	{
		m_task.run(get<index>()...);
	}

	template <size_t index>
	auto get()
	{
		auto queue = std::get<index>(m_input);
		auto value = queue.front();
		queue.pop();

		return value;
	}
	
	template <size_t index = 0, typename T = typename Helper::ReturnType, typename Output = std::enable_if_t<std::is_same<T, void>::value, T>>
	std::enable_if_t<index < Helper::OutputParameterCount> write(Output &output)
	{
		auto &value = std::get<index>(output);
		for (auto &callback : std::get<index>(m_output))
		{
			callback(value);
		}

		write<index + 1>(output);
	}

	template <size_t index, typename T = typename Helper::ReturnType, typename Output = std::enable_if_t<std::is_same<T, void>::value, T>>
	std::enable_if_t<index == Helper::OutputParameterCount> write(Output &)
	{
	}

	typename Helper::InputQueue m_input;
	typename Helper::ReturnCallbacks m_output;
	Task m_task;
};

} // namespace yats
