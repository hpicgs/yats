#pragma once

#include <functional>
#include <memory>
#include <queue>
#include <tuple>
#include <vector>

#include <yats/output_connector.h>
#include <yats/slot.h>

namespace yats
{

template <typename T>
struct output_wrapper;

template <typename... ParameterTypes>
struct output_wrapper<std::tuple<ParameterTypes...>>
{
    template <typename CompoundType>
    static std::vector<std::function<void(typename CompoundType::value_type)>> transform_callback();

    template <typename CompoundType>
    static output_connector<typename CompoundType::value_type> transform_connector();

    using callbacks = std::tuple<decltype(transform_callback<ParameterTypes>())...>;
    using connectors = std::tuple<decltype(transform_connector<ParameterTypes>())...>;
    using tuple = std::tuple<ParameterTypes...>;

    static constexpr size_t parameter_count = sizeof...(ParameterTypes);
};

template <typename T, uint64_t Id>
struct output_wrapper<slot<T, Id>>
{
    using callbacks = std::tuple<std::vector<std::function<void(T)>>>;
    using connectors = std::tuple<output_connector<T>>;
    using tuple = std::tuple<slot<T, Id>>;

    static constexpr size_t parameter_count = 1;
};

template <>
struct output_wrapper<void>
{
    using callbacks = std::tuple<>;
    using connectors = std::tuple<>;
    using tuple = std::tuple<>;

    static constexpr size_t parameter_count = 0;
};

template <typename Return, typename... ParameterTypes>
struct task_helper
{
    template <typename CompoundType>
    static std::queue<typename CompoundType::value_type> transform_queue();

    template <typename CompoundType>
    static std::function<void(typename CompoundType::value_type)> transform_callback();

    template <typename CompoundType>
    static input_connector<typename CompoundType::value_type> transform_connector();

    using input_queue = std::tuple<decltype(transform_queue<ParameterTypes>())...>;
    using input_queue_ptr = std::unique_ptr<input_queue>;

    using basic_output_type = Return;

    using input_tuple = std::tuple<ParameterTypes...>;
    using output_tuple = typename output_wrapper<Return>::tuple;

    using input_callbacks = std::tuple<decltype(transform_callback<ParameterTypes>())...>;
    using output_callbacks = typename output_wrapper<Return>::callbacks;

    using input_connectors = std::tuple<decltype(transform_connector<ParameterTypes>())...>;
    using output_connectors = typename output_wrapper<Return>::connectors;

    static constexpr size_t input_count = sizeof...(ParameterTypes);
    static constexpr size_t output_count = output_wrapper<Return>::parameter_count;
};

template <typename ReturnType, typename TaskType, typename... ParameterTypes>
static constexpr task_helper<ReturnType, ParameterTypes...> make_helper(ReturnType (TaskType::*)(ParameterTypes...));
}
