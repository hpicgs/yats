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

template<uint64_t Id, typename T>
struct get_element_by_id
{
	template<typename... Args>
	struct Parser;

	template<typename First, typename... Args>
	struct Parser<First, Args...>
	{
		template<uint64_t LocalId = Id, typename F = First>
		static constexpr decltype(std::enable_if_t<F::ID != LocalId, Parser<Args...>>::parse_type()) parse_type();

		template<uint64_t LocalId = Id, typename F = First>
		static constexpr typename std::enable_if_t<F::ID == LocalId, F>::value_type parse_type();
	};

	template<typename First>
	struct Parser<First>
	{
		template<uint64_t LocalId = Id, typename F = First>
		static constexpr typename std::enable_if_t<F::ID == LocalId, F>::value_type parse_type();
	};

	template<typename... Args>
	static constexpr Parser<Args...> Help(std::tuple<Args...>);

	template<size_t Index = 0, typename Tuple = T>
	static constexpr std::enable_if_t<Index < std::tuple_size<Tuple>::value, size_t> outer_parse_index()
	{
		return inner_parse_index<Index>();
	}

	template<size_t Index = 0, typename Tuple = T>
	static constexpr std::enable_if_t<Index == std::tuple_size<Tuple>::value, size_t> outer_parse_index()
	{
		return 0;
	}

	template<size_t Index, uint64_t LocalId = Id, typename Tuple = T>
	static constexpr std::enable_if_t<std::tuple_element<Index, Tuple>::type::ID == LocalId, size_t> inner_parse_index()
	{
		return Index + outer_parse_index<Index + 1>();
	}

	template<size_t Index = 0, uint64_t LocalId = Id, typename Tuple = T>
	static constexpr std::enable_if_t<std::tuple_element<Index, Tuple>::type::ID != LocalId, size_t> inner_parse_index()
	{
		return outer_parse_index<Index + 1>();
	}

	using type = decltype(decltype(Help(std::declval<T>()))::parse_type());
	static constexpr uint64_t index = outer_parse_index();
};

template<uint64_t Id, typename T>
using get_element_by_id_t = typename get_element_by_id<Id, T>::type;

}  // namespace yats
