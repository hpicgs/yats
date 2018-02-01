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

TEST(pipeline_test, add_one_listener)
{
    yats::pipeline pipeline;

    auto lambda_source = pipeline.add([]() -> yats::slot<int, 0> { return 30; });

    int output = 0;
    lambda_source->add_listener<0>([&output](int value) { output = value; });

    yats::scheduler scheduler(pipeline);
    scheduler.run();

    EXPECT_EQ(output, 30);
}

TEST(pipeline_test, add_listener_and_task)
{
    yats::pipeline pipeline;

    auto lambda_source = pipeline.add([]() -> yats::slot<int, 0> { return 30; });
    auto lambda_target = pipeline.add([](yats::slot<int, 0>) {});

    int output = 0;
    lambda_source->add_listener<0>([&output](int value) { output = value; });

    lambda_source->output<0>() >> lambda_target->input<0>();

    yats::scheduler scheduler(pipeline);
    scheduler.run();

    EXPECT_EQ(output, 30);
}

TEST(pipeline_test, add_no_listener)
{
    yats::pipeline pipeline;

    auto lambda_source = pipeline.add([]() -> yats::slot<int, 0> { return 30; });
    (void) lambda_source;

    yats::scheduler scheduler(pipeline);
    scheduler.run();
}

TEST(pipeline_test, add_multiple_listener)
{
    yats::pipeline pipeline;

    auto lambda_source = pipeline.add([]() -> yats::slot<int, 0> { return 30; });

    int output1 = 0;
    int output2 = 0;
    lambda_source->add_listener<0>([&output1](int value) { output1 = value; });
    lambda_source->add_listener<0>([&output2](int value) { output2 = value; });

    yats::scheduler scheduler(pipeline);
    scheduler.run();

    EXPECT_EQ(output1, 30);
    EXPECT_EQ(output2, 30);
}

TEST(pipeline_test, add_initial_input)
{
    yats::pipeline pipeline;

    int value_after_add = 0;
    int value_after_subtract = 0;

    auto add = pipeline.add([&value_after_add](yats::slot<int, 0> value) -> yats::slot<int, 0>
    {
        value_after_add = value + 10;
        return value_after_add;
    });
    auto subtract = pipeline.add([&value_after_subtract](yats::slot<int, 0> value) -> yats::slot<int, 0>
    {
        value_after_subtract = value - 20;
        return value_after_subtract;
    });

    add->output<0>() >> subtract->input<0>();
    subtract->output<0>() >> add->input<0>();

    add->add_input_value<0>(10);

    yats::scheduler scheduler(pipeline);
    scheduler.run();

    EXPECT_EQ(value_after_add, 20);
    EXPECT_EQ(value_after_subtract, 0);

    scheduler.run();

    EXPECT_EQ(value_after_add, 10);
    EXPECT_EQ(value_after_subtract, -10);
}
