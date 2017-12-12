#pragma once

#include <functional>
#include <memory>
#include <queue>
#include <vector>

namespace yats
{

template<typename T>
struct has_run
{
	template<typename U> static char test_function(decltype(&U::run));
	template<typename U> static int test_function(...);
	static constexpr bool value = sizeof(test_function<T>(0)) == sizeof(char);
};

template<typename T>
constexpr bool has_run_v = has_run<T>::value;


template<typename T>
struct is_unique_ptr
{
	template<typename U> static char test_function(const std::unique_ptr<U> &);
	static int test_function(...);
	static constexpr bool value = sizeof(test_function(std::declval<T>())) == sizeof(char);
};

template<>
struct is_unique_ptr<void>
{
	static constexpr bool value = false;
};

template<typename T>
constexpr bool is_unique_ptr_v = is_unique_ptr<T>::value;


template<typename T>
struct is_shared_ptr
{
	template<typename U> static char test_function(const std::shared_ptr<U> &);
	static int test_function(...);
	static constexpr bool value = sizeof(test_function(std::declval<T>())) == sizeof(char);
};

template<>
struct is_shared_ptr<void>
{
	static constexpr bool value = false;
};

template<typename T>
constexpr bool is_shared_ptr_v = is_shared_ptr<T>::value;


template<typename T>
struct ReturnWrapper;

template<typename... ParameterTypes>
struct ReturnWrapper<std::tuple<ParameterTypes...>>
{
	template <typename CompoundType>
	static std::vector<std::function<void(typename CompoundType::value_type)>> transform_callback();

	using Callbacks = std::tuple<decltype(transform_callback<ParameterTypes>())...>;

	static constexpr size_t ParameterCount = sizeof...(ParameterTypes);
};

template<>
struct ReturnWrapper<void>
{
	using Callbacks = std::tuple<>;

	static constexpr size_t ParameterCount = 0;
};

template <typename Return, typename... ParameterTypes>
struct TaskHelper
{
	template <typename CompoundType>
	static typename CompoundType::value_type transform_base();

	template <typename CompoundType>
	static std::queue<typename CompoundType::value_type> transform_queue();

	using WrappedInput = std::tuple<ParameterTypes...>;
	using Input = std::tuple<decltype(transform_base<ParameterTypes>())...>;
	using InputQueue = std::tuple<decltype(transform_queue<ParameterTypes>())...>;
	using ReturnType = Return;

	using ReturnCallbacks = typename ReturnWrapper<ReturnType>::Callbacks;

	static constexpr size_t ParameterCount = sizeof...(ParameterTypes);
	static constexpr size_t OutputParameterCount = typename ReturnWrapper<ReturnType>::ParameterCount;
};

template <typename ReturnType, typename TaskType, typename... ParameterTypes>
static constexpr TaskHelper<ReturnType, ParameterTypes...> MakeHelper(ReturnType(TaskType::*)(ParameterTypes...));

}  // namespace yats
