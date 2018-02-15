#include <memory>

#include <test_util.h>

#include <gmock/gmock.h>

#include <yats/scheduler.h>
#include <yats/slot.h>
#include <yats/task_container.h>

using namespace yats;

TEST(task_container_test, perfect_forward_return_values)
{
    pipeline pipeline;

    int test_int = 25;
    auto lambda_source = pipeline.add([]() -> slot<std::unique_ptr<int>, 1> { return std::make_unique<int>(30); });
    auto lambda_target = pipeline.add([&test_int](slot<std::unique_ptr<int>, 1> value) mutable { test_int = **value; });

    lambda_source->output<1>() >> lambda_target->input<1>();

    scheduler scheduler(std::move(pipeline));
    scheduler.run();

    EXPECT_EQ(test_int, 30);
}

TEST(task_container_test, copy_once)
{
    pipeline pipeline;

    auto lambda_source = pipeline.add([]() -> slot<constructor_counter, 1> { return constructor_counter(); });

    int copy_counter_1 = 10;
    auto lambda_target_1 = pipeline.add([&copy_counter_1](slot<constructor_counter, 1> value) mutable { copy_counter_1 = value->copied; });
    lambda_source->output<1>() >> lambda_target_1->input<1>();

    int copy_counter_2 = 10;
    auto lambda_target_2 = pipeline.add([&copy_counter_2](slot<constructor_counter, 1> value) mutable { copy_counter_2 = value->copied; });
    lambda_source->output<1>() >> lambda_target_2->input<1>();

    scheduler scheduler(std::move(pipeline));
    scheduler.run();

    EXPECT_EQ(copy_counter_1 + copy_counter_2, 1);
}
