#pragma once

#include <map>
#include <memory>
#include <vector>

#include <yats/constraint.h>
#include <yats/task_configurator.h>

namespace yats
{

class thread_group_helper
{
public:
    static std::map<std::string, size_t> map_thread_groups(const std::vector<std::unique_ptr<abstract_task_configurator>>& tasks)
    {
        std::map<std::string, size_t> constraint_map;
        for (size_t group = 0; group < thread_group::COUNT; ++group)
        {
            constraint_map.emplace(thread_group::name_for(thread_group::given_groups(group)), group);
        }

        size_t next_constraint = thread_group::given_groups::COUNT;
        for (const auto& task : tasks)
        {
            for (const auto& constraint_name : task->thread_constraints().names())
            {
                auto success = constraint_map.emplace(constraint_name, next_constraint);
                if (success.second)
                {
                    ++next_constraint;
                }
            }
        }

        return constraint_map;
    }
};
}
