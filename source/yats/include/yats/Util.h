#pragma once

#include <memory>

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


template <typename Return, typename... ParameterTypes>
struct TaskHelper
{
	template <typename CompoundType>
	static typename CompoundType::value_type transform();

	using WrappedInput = std::tuple<ParameterTypes...>;
	using Input = std::tuple<decltype(transform<ParameterTypes>())...>;
	using ReturnType = Return;

	static constexpr size_t ParameterCount = sizeof...(ParameterTypes);
};

template <typename ReturnType, typename TaskType, typename... ParameterTypes>
static constexpr TaskHelper<ReturnType, ParameterTypes...> MakeHelper(ReturnType(TaskType::*)(ParameterTypes...));

template<uint64_t Id, typename... Args>
struct Parser;

template<uint64_t Id, typename First, typename... Args>
struct Parser<Id, First, Args...>
{
	template<uint64_t LocalId = Id, typename F = First, typename Dummy = std::enable_if_t<F::ID != LocalId, F>>
	static constexpr decltype(Parser<Id, Args...>::parse()) parse();

	template<uint64_t LocalId = Id, typename F = First, typename Dummy = std::enable_if_t<F::ID == LocalId, F>>
	static constexpr typename F::value_type parse();
};

template<uint64_t Id, typename First>
struct Parser<Id, First>
{
	template<uint64_t LocalId = Id, typename F = First, typename Dummy = std::enable_if_t<F::ID == LocalId, F>>
	static constexpr typename F::value_type parse();
};

}  // namespace yats
