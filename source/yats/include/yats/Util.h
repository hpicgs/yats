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
struct get_index_by_id
{
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

	static constexpr uint64_t value = outer_parse_index();
};

template<uint64_t Id, typename T>
static constexpr size_t get_index_by_id_v = get_index_by_id<Id, T>::value;


template<typename T>
struct has_unique_ids
{
	static constexpr bool check_ids()
	{
		constexpr size_t size = std::tuple_size<T>::value;
		uint64_t ids[size] = {};
		write(ids);

		for (size_t i = 0; i < size; ++i)
		{
			for (size_t j = i + 1; j < size; ++j)
			{
				if (ids[i] == ids[j])
				{
					return false;
				}
			}
		}
		return true;
	}

	template<uint64_t Index = 0>
	static constexpr std::enable_if_t<Index < std::tuple_size<T>::value, void> write(uint64_t* ids)
	{
		ids[Index] = std::tuple_element_t<Index, T>::ID;
		write<Index + 1>(ids);
	}

	template<uint64_t Index>
	static constexpr std::enable_if_t<Index == std::tuple_size<T>::value, void> write(uint64_t*)
	{
	}

	static constexpr bool value = check_ids();
};

template<typename T>
static constexpr bool has_unique_ids_v = has_unique_ids<T>::value;

}  // namespace yats
