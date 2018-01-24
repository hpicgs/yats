#pragma once

#include <thread>
#include <vector>

namespace yats
{

class threadpool
{
public:
protected:
    std::vector<std::thread> m_threads;
};
}
