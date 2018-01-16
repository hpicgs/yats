#include <gmock/gmock.h>

#include <yats/pipeline.h>
#include <yats/scheduler.h>
#include <yats/slot.h>

TEST(pipeline_test, add_lambda_task)
{
    yats::pipeline pipeline;

    int ss = 25;
    auto lambda_source = pipeline.add([&ss]() -> yats::slot<int, 1> { return 30; });
    auto lambda_target = pipeline.add([&ss](yats::slot<int, 1> s) mutable { ss = s; });

    lambda_source->output<1>() >> lambda_target->input<1>();

    yats::scheduler scheduler(pipeline);
    scheduler.run();

    EXPECT_EQ(ss, 30);
}
