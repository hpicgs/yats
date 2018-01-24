#include <gmock/gmock.h>

#include <yats/slot.h>
#include <yats/task_container.h>
#include <yats/scheduler.h>

#include <memory>

TEST(taskcontainer_test, return_parameters)
{
    yats::pipeline pipeline;

    int test_int = 25;
    auto lambda_source = pipeline.add([]() -> yats::slot<std::unique_ptr<int>, 1> { return std::make_unique<int>(30); });
    auto lambda_target = pipeline.add([&test_int](yats::slot<std::unique_ptr<int>, 1> value) mutable { test_int = **value; });

    lambda_source->output<1>() >> lambda_target->input<1>();

    //yats::scheduler scheduler(pipeline);
    //scheduler.run();

    //EXPECT_EQ(test_int, 30);
}
