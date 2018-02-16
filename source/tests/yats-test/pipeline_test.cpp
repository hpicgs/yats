#include <gmock/gmock.h>

#include <yats/pipeline.h>
#include <yats/scheduler.h>
#include <yats/slot.h>

using namespace yats;

TEST(pipeline_test, add_lambda_task)
{
    pipeline pipeline;

    int test_int = 25;
    auto lambda_source = pipeline.add([]() -> slot<int, 1> { return 30; });
    auto lambda_target = pipeline.add([&test_int](slot<int, 1> value) mutable { test_int = value; });

    lambda_source->output<1>() >> lambda_target->input<1>();

    scheduler scheduler(std::move(pipeline));
    scheduler.run();

    EXPECT_EQ(test_int, 30);
}

TEST(pipeline_test, add_one_listener)
{
    pipeline pipeline;

    auto lambda_source = pipeline.add([]() -> slot<int, 0> { return 30; });

    int output = 0;
    lambda_source->add_listener<0>([&output](int value) { output = value; });

    scheduler scheduler(std::move(pipeline));
    scheduler.run();

    EXPECT_EQ(output, 30);
}

TEST(pipeline_test, add_listener_and_task)
{
    pipeline pipeline;

    auto lambda_source = pipeline.add([]() -> slot<int, 0> { return 30; });
    auto lambda_target = pipeline.add([](slot<int, 0>) {});

    int output = 0;
    lambda_source->add_listener<0>([&output](int value) { output = value; });

    lambda_source->output<0>() >> lambda_target->input<0>();

    scheduler scheduler(std::move(pipeline));
    scheduler.run();

    EXPECT_EQ(output, 30);
}

TEST(pipeline_test, add_no_listener)
{
    pipeline pipeline;

    auto lambda_source = pipeline.add([]() -> slot<int, 0> { return 30; });
    (void) lambda_source;

    scheduler scheduler(std::move(pipeline));
    scheduler.run();
}

TEST(pipeline_test, add_multiple_listener)
{
    pipeline pipeline;

    auto lambda_source = pipeline.add([]() -> slot<int, 0> { return 30; });

    int output1 = 0;
    int output2 = 0;
    lambda_source->add_listener<0>([&output1](int value) { output1 = value; });
    lambda_source->add_listener<0>([&output2](int value) { output2 = value; });

    scheduler scheduler(std::move(pipeline));
    scheduler.run();

    EXPECT_EQ(output1, 30);
    EXPECT_EQ(output2, 30);
}
