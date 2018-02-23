#pragma once

#include <numeric>
#include <memory>
#include <condition_variable>
#include <mutex>

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
struct has_options
{
    template <typename U>
    static char test_function(decltype(&U::options));
    template <typename U>
    static int test_function(...);
    static constexpr bool value = sizeof(test_function<T>(0)) == sizeof(char);
};

template <typename T>
constexpr bool has_options_v = has_options<T>::value;

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
struct is_tuple
{
    template <typename... U>
    static char test_function(const std::tuple<U...>&);
    static int test_function(...);
    static constexpr bool value = sizeof(test_function(std::declval<T>())) == sizeof(char);
};

template <>
struct is_tuple<void>
{
    static constexpr bool value = false;
};

template <typename T>
constexpr bool is_tuple_v = is_tuple<T>::value;

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

class condition
{
public:
    condition(size_t number_of_threads)
        : m_is_active(true)
        , m_number_of_threads(number_of_threads)
        , m_notify_count({ {"thread", number_of_threads } })
    {
    }

    class thread_guard
    {
    public:
        thread_guard(condition *condition, bool is_active, std::unique_lock<std::mutex> &guard)
            : m_condition(condition)
            , m_is_active(is_active)
        {
            // Wait on the resource condition.
            while (condition->m_notify_count["thread"] == 0 && m_is_active)
            {
                condition->m_task_added["thread"].wait(guard);
            }
            --condition->m_notify_count["thread"];
        }

        ~thread_guard()
        {
            m_condition->notify("thread");
            if (m_is_active)
            {
                m_condition->check_finish();
            }
        }

        operator bool() const
        {
            return m_is_active;
        }

    protected:
        condition *m_condition;
        const bool m_is_active;
    };

    thread_guard wait(const std::string& constraint)
    {
        std::unique_lock<std::mutex> guard(m_mutex);

        // Wait on the resource condition.
        while (m_notify_count[constraint] == 0 && m_is_active)
        {
            m_task_added[constraint].wait(guard);
        }
        --m_notify_count[constraint];

        return thread_guard(this, m_is_active, guard);
    }

    thread_guard wait_main(const std::string& constraint)
    {
        std::unique_lock<std::mutex> guard(m_mutex);

        // Wait on the resource condition.
        while (m_notify_count[constraint] == 0 && m_is_active)
        {
            m_task_added[constraint].wait(guard);
        }
        --m_notify_count[constraint];

        return thread_guard(this, !has_finished(), guard);
    }

    void notify(const std::string& constraint)
    {
        std::unique_lock<std::mutex> guard(m_mutex);
        ++m_notify_count[constraint];
        m_task_added[constraint].notify_one();
    }

    void check_finish()
    {
        std::unique_lock<std::mutex> guard(m_mutex);
        if (has_finished() && m_notify_count["main"] == 0)
        {
            std::cout << "checked finish:" << std::this_thread::get_id() << std::endl;
            ++m_notify_count["main"];
            m_task_added["main"].notify_one();
        }
    }

    void terminate()
    {
        std::unique_lock<std::mutex> guard(m_mutex);
        m_is_active = false;
        for (auto& elem : m_task_added)
        {
            elem.second.notify_all();
        }
    }

    bool is_active() const
    {
        return m_is_active;
    }

    bool has_finished()
    {
        auto count = std::accumulate(m_notify_count.cbegin(), m_notify_count.cend(), 0ull, [](size_t count, const auto& node)
        {
            return node.second + count;
        });
        return count == m_number_of_threads && m_notify_count["thread"] == m_number_of_threads;
    }

//protected:
    //void reserve(const std::string& constraint, std::unique_lock<std::mutex>& guard)
    //{
    //    std::unique_lock<std::mutex> guard(m_mutex);
    //    // Wait on the resource condition.
    //    while (m_notify_count[constraint] == 0 && m_is_active)
    //    {
    //        m_task_added[constraint].wait(guard);
    //    }
    //    --m_notify_count[constraint];
    //}

    bool m_is_active;
    std::mutex m_mutex;
    std::map<std::string, std::condition_variable> m_task_added;
    std::map<std::string, size_t> m_notify_count;
    size_t m_number_of_threads;

};
}
