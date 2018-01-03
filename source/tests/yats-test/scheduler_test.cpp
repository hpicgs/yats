#include <gmock/gmock.h>

#include <yats/Scheduler.h>
#include <yats/Util.h>

TEST(scheduler_test, simple_create)
{
    std::vector<std::unique_ptr<yats::AbstractTaskConfigurator>> empty_task_configs;
    EXPECT_NO_THROW(yats::Scheduler scheduler{ empty_task_configs });
}
