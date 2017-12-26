#include <gmock/gmock.h>

#include <yats/Util.h>
#include <yats/Scheduler.h>


TEST(scheduler_test, simple_create)
{
	std::map<std::string, std::unique_ptr<yats::AbstractTaskConfigurator>> empty_task_configs;
	EXPECT_NO_THROW(yats::scheduler scheduler{empty_task_configs});
}
