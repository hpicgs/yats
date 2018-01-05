#include <gmock/gmock.h>

#include <yats/scheduler.h>
#include <yats/util.h>

TEST(scheduler_test, simple_create)
{
    std::vector<std::unique_ptr<yats::abstract_task_configurator>> empty_task_configs;
    EXPECT_NO_THROW(yats::scheduler scheduler{ empty_task_configs });
}
