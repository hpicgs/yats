#pragma once

#include <mutex>
#include <queue>

namespace yats
{

template <typename ValueType>
class queue
{
public:
    using lock = std::lock_guard<std::mutex>;

    queue() = default;

    ValueType extract()
    {
        lock guard(m_mutex);
        auto value = std::move(m_queue.front());
        m_queue.pop();
        return value;
    }

    void push(ValueType value)
    {
        lock guard(m_mutex);
        m_queue.push(std::move(value));
    }

    size_t size()
    {
        lock guard(m_mutex);
        return m_queue.size();
    }

protected:
    std::queue<ValueType> m_queue;
    std::mutex m_mutex;
};
}
