#include <gmock/gmock.h>

#include <yats/scheduler.h>
#include <yats/util.h>

TEST(scheduler_test, simple_create)
{
    std::vector<std::unique_ptr<yats::AbstractTaskConfigurator>> empty_task_configs;
    EXPECT_NO_THROW(yats::Scheduler scheduler{ empty_task_configs });
}
