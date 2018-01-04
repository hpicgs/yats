#pragma once

#include <array>
#include <functional>
#include <memory>
#include <queue>
#include <vector>

#include <yats/output_connector.h>

namespace yats
{

template <typename T>
struct has_run
{
    template <typename U>
    static char test_function(decltype(&U::run));
    template <typename U>
    static int test_function(...);
    static constexpr bool value = sizeof(test_function<T>(0)) == sizeof(char);
};

template <typename T>
constexpr bool has_run_v = has_run<T>::value;

template <typename T>
struct is_unique_ptr
{
    template <typename U>
    static char test_function(const std::unique_ptr<U>&);
    static int test_function(...);
    static constexpr bool value = sizeof(test_function(std::declval<T>())) == sizeof(char);
};

template <>
struct is_unique_ptr<void>
{
    static constexpr bool value = false;
};

template <typename T>
constexpr bool is_unique_ptr_v = is_unique_ptr<T>::value;

template <typename T>
struct is_shared_ptr
{
    template <typename U>
    static char test_function(const std::shared_ptr<U>&);
    static int test_function(...);
    static constexpr bool value = sizeof(test_function(std::declval<T>())) == sizeof(char);
};

template <>
struct is_shared_ptr<void>
{
    static constexpr bool value = false;
};

template <typename T>
constexpr bool is_shared_ptr_v = is_shared_ptr<T>::value;

template <typename T>
struct return_wrapper;

template <typename... ParameterTypes>
struct return_wrapper<std::tuple<ParameterTypes...>>
{
    template <typename CompoundType>
    static std::vector<std::function<void(typename CompoundType::value_type)>> transform_callback();

    using callbacks = std::tuple<decltype(transform_callback<ParameterTypes>())...>;

    template <typename CompoundType>
    static output_connector<typename CompoundType::value_type> transform_output();

    using output_configuration = std::tuple<decltype(transform_output<ParameterTypes>())...>;

    template <typename CompoundType>
    static CompoundType transform_base();

    using base = std::tuple<ParameterTypes...>;

    static constexpr size_t parameter_count = sizeof...(ParameterTypes);
};

template <>
struct return_wrapper<void>
{
    using callbacks = std::tuple<>;
    using output_configuration = std::tuple<>;
    using base = std::tuple<>;

    static constexpr size_t parameter_count = 0;
};

template <typename Return, typename... ParameterTypes>
struct task_helper
{
    template <typename CompoundType>
    static typename CompoundType::value_type transform_base();

    template <typename CompoundType>
    static std::queue<typename CompoundType::value_type> transform_queue();

    template <typename CompoundType>
    static std::function<void(typename CompoundType::value_type)> transform_function();

    using wrapped_input = std::tuple<ParameterTypes...>;
    using input = std::tuple<decltype(transform_base<ParameterTypes>())...>;
    using input_queue_base = std::tuple<decltype(transform_queue<ParameterTypes>())...>;
    using input_queue = std::unique_ptr<input_queue_base>;
    using return_type = Return;

    using return_base = typename return_wrapper<return_type>::base;

    using return_callbacks = typename return_wrapper<return_type>::callbacks;
    using input_callbacks = std::tuple<decltype(transform_function<ParameterTypes>())...>;

    template <typename CompoundType>
    static input_connector<typename CompoundType::value_type> transform_input();

    using input_configuration = std::tuple<decltype(transform_input<ParameterTypes>())...>;
    using output_configuration = typename return_wrapper<return_type>::output_configuration;

    static constexpr size_t parameter_count = sizeof...(ParameterTypes);
    static constexpr size_t output_parameter_count = return_wrapper<return_type>::parameter_count;
};

template <typename ReturnType, typename TaskType, typename... ParameterTypes>
static constexpr task_helper<ReturnType, ParameterTypes...> make_helper(ReturnType (TaskType::*)(ParameterTypes...));

template <uint64_t Id, typename T>
struct get_index_by_id
{
    static constexpr size_t invalid_index = std::numeric_limits<size_t>::max();

    template <size_t Index = 0, typename Tuple = T>
    static constexpr std::enable_if_t<(Index < std::tuple_size<Tuple>::value), size_t> find()
    {
        size_t index_id = std::tuple_element_t<Index, Tuple>::id;
        if (Id == index_id)
        {
            return Index;
        }
        return find<Index + 1>();
    }

    template <size_t Index = 0, typename Tuple = T>
    static constexpr std::enable_if_t<Index == std::tuple_size<Tuple>::value, size_t> find()
    {
        return invalid_index;
    }

    static constexpr uint64_t value = find();
    static_assert(value != invalid_index, "Could not find identifier in tuple.");
};

template <uint64_t Id, typename T>
static constexpr size_t get_index_by_id_v = get_index_by_id<Id, T>::value;

template <typename T>
struct has_unique_ids
{
    template <typename Tuple = T, size_t TupleSize = std::tuple_size<Tuple>::value>
    static constexpr std::enable_if_t<(TupleSize > 0), bool> check_ids()
    {
        uint64_t ids[TupleSize] = {};
        write(ids);

        for (size_t i = 0; i < TupleSize; ++i)
        {
            for (size_t j = i + 1; j < TupleSize; ++j)
            {
                if (ids[i] == ids[j])
                {
                    return false;
                }
            }
        }
        return true;
    }

    template <typename Tuple = T, size_t TupleSize = std::tuple_size<Tuple>::value>
    static constexpr std::enable_if_t<(TupleSize == 0), bool> check_ids()
    {
        return true;
    }

    template <uint64_t Index = 0, size_t TupleSize = std::tuple_size<T>::value>
    static constexpr std::enable_if_t<(Index < TupleSize)> write(uint64_t* ids)
    {
        ids[Index] = std::tuple_element_t<Index, T>::id;
        write<Index + 1>(ids);
    }

    template <uint64_t Index, size_t TupleSize = std::tuple_size<T>::value>
    static constexpr std::enable_if_t<Index == TupleSize> write(uint64_t*)
    {
    }

    static constexpr bool value = check_ids();
};

template <typename T>
static constexpr bool has_unique_ids_v = has_unique_ids<T>::value;

// Taken from http://en.cppreference.com/w/cpp/utility/make_from_tuple
namespace detail
{
    template <class T, class Tuple, std::size_t... I>
    constexpr T make_from_tuple_impl(Tuple&& t, std::index_sequence<I...>)
    {
        (void) t;
        return T(std::get<I>(std::forward<Tuple>(t))...);
    }
}

template <class T, class Tuple>
constexpr T make_from_tuple(Tuple&& t)
{
    return detail::make_from_tuple_impl<T>(std::forward<Tuple>(t), std::make_index_sequence<std::tuple_size<std::remove_reference_t<Tuple>>::value>{});
}
}
