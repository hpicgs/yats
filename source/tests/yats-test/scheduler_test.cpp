#include <gmock/gmock.h>

#include <yats/scheduler.h>
#include <yats/util.h>

TEST(scheduler_test, simple_create)
{
    yats::pipeline empty_task_configs;
    EXPECT_NO_THROW(yats::scheduler{ empty_task_configs });
}
