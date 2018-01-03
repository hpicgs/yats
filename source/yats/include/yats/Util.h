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
struct ReturnWrapper;

template <typename... ParameterTypes>
struct ReturnWrapper<std::tuple<ParameterTypes...>>
{
    template <typename CompoundType>
    static std::vector<std::function<void(typename CompoundType::value_type)>> transform_callback();

    using Callbacks = std::tuple<decltype(transform_callback<ParameterTypes>())...>;

    template <typename CompoundType>
    static output_connector<typename CompoundType::value_type> transform_output();

    using OutputConfiguration = std::tuple<decltype(transform_output<ParameterTypes>())...>;

    template <typename CompoundType>
    static CompoundType transform_base();

    using Base = std::tuple<ParameterTypes...>;

    static constexpr size_t ParameterCount = sizeof...(ParameterTypes);
};

template <>
struct ReturnWrapper<void>
{
    using Callbacks = std::tuple<>;
    using OutputConfiguration = std::tuple<>;
    using Base = std::tuple<>;

    static constexpr size_t ParameterCount = 0;
};

template <typename Return, typename... ParameterTypes>
struct TaskHelper
{
    template <typename CompoundType>
    static typename CompoundType::value_type transform_base();

    template <typename CompoundType>
    static std::queue<typename CompoundType::value_type> transform_queue();

    template <typename CompoundType>
    static std::function<void(typename CompoundType::value_type)> transform_function();

    using WrappedInput = std::tuple<ParameterTypes...>;
    using Input = std::tuple<decltype(transform_base<ParameterTypes>())...>;
    using InputQueueBase = std::tuple<decltype(transform_queue<ParameterTypes>())...>;
    using InputQueue = std::unique_ptr<InputQueueBase>;
    using ReturnType = Return;

    using ReturnBase = typename ReturnWrapper<ReturnType>::Base;

    using ReturnCallbacks = typename ReturnWrapper<ReturnType>::Callbacks;
    using InputCallbacks = std::tuple<decltype(transform_function<ParameterTypes>())...>;

    template <typename CompoundType>
    static input_connector<typename CompoundType::value_type> transform_input();

    using InputConfiguration = std::tuple<decltype(transform_input<ParameterTypes>())...>;
    using OutputConfiguration = typename ReturnWrapper<ReturnType>::OutputConfiguration;

    static constexpr size_t ParameterCount = sizeof...(ParameterTypes);
    static constexpr size_t OutputParameterCount = ReturnWrapper<ReturnType>::ParameterCount;
};

template <typename ReturnType, typename TaskType, typename... ParameterTypes>
static constexpr TaskHelper<ReturnType, ParameterTypes...> MakeHelper(ReturnType (TaskType::*)(ParameterTypes...));

template <uint64_t Id, typename T>
struct get_index_by_id
{
    static constexpr size_t invalid_index = std::numeric_limits<size_t>::max();

    template <size_t Index = 0, typename Tuple = T>
        static constexpr std::enable_if_t < Index<std::tuple_size<Tuple>::value, size_t> find()
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
        static constexpr std::enable_if_t < Index<TupleSize> write(uint64_t* ids)
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
}
