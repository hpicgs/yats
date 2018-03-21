#pragma once

#include <mutex>
#include <queue>

namespace yats
{

template <typename ValueType>
class thread_safe_queue
{
    using lock = std::lock_guard<std::mutex>;

public:
    using value_type = ValueType;

    ValueType extract()
    {
        lock guard(m_mutex);
        auto value = std::move(m_queue.front());
        m_queue.pop();
        --m_num_reserved;
        return value;
    }

    void push(ValueType&& value)
    {
        lock guard(m_mutex);
        m_queue.push(std::forward<ValueType>(value));
    }

    void push(const ValueType& value)
    {
        lock guard(m_mutex);
        m_queue.push(value);
    }

    bool empty()
    {
        lock guard(m_mutex);
        return m_queue.empty() || m_queue.size() == m_num_reserved;
    }

    void reserve_one()
    {
        lock guard(m_mutex);
        if (m_num_reserved == m_queue.size())
        {
            throw std::runtime_error("Trying to reserve more elements than the queue currently holds.");
        }
        ++m_num_reserved;
    }

protected:
    std::queue<ValueType> m_queue;
    std::mutex m_mutex;
    size_t m_num_reserved;
};
}
