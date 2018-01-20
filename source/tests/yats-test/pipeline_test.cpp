#include <gmock/gmock.h>

#include <yats/pipeline.h>
#include <yats/scheduler.h>
#include <yats/slot.h>

TEST(pipeline_test, add_lambda_task)
{
    yats::pipeline pipeline;

    int test_int = 25;
    auto lambda_source = pipeline.add([]() -> yats::slot<int, 1> { return 30; });
    auto lambda_target = pipeline.add([&test_int](yats::slot<int, 1> value) mutable { test_int = value; });

    lambda_source->output<1>() >> lambda_target->input<1>();

    yats::scheduler scheduler(pipeline);
    scheduler.run();

    EXPECT_EQ(test_int, 30);
}
