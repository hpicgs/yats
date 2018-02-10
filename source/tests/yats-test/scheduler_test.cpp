#include <gmock/gmock.h>

#include <yats/scheduler.h>
#include <yats/slot.h>
#include <yats/thread_pool.h>

TEST(scheduler_test, simple_create)
{
    yats::pipeline empty_task_configs;
    EXPECT_NO_THROW(yats::scheduler{ empty_task_configs });
}

TEST(scheduler_test, multithreaded_timing_test)
{
    yats::pipeline pipeline;

    auto function = []()
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    };

    pipeline.add(function);
    pipeline.add(function);
    pipeline.add(function);

    yats::scheduler scheduler(pipeline);
    auto start = std::chrono::high_resolution_clock::now();
    scheduler.run();
    auto end = std::chrono::high_resolution_clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    EXPECT_GE(duration, 100);
    EXPECT_LE(duration, 500);
}

TEST(scheduler_test, run_twice)
{
    yats::pipeline pipeline;

    int output = 0;
    auto task = pipeline.add([]() -> yats::slot<int, 0> { return 1; });
    task->add_listener<0>([&output](int value) {output += value; });

    yats::scheduler scheduler(pipeline);
    EXPECT_EQ(output, 0);
    scheduler.run();
    EXPECT_EQ(output, 1);
    scheduler.run();
    EXPECT_EQ(output, 2);
}

TEST(scheduler_test, throw_on_creation)
{
    yats::pipeline pipeline;

    auto source = pipeline.add([]() -> yats::slot<std::unique_ptr<int>, 0> { return std::make_unique<int>(0); });
    source->add_listener<0>([](std::unique_ptr<int>) {});
    source->add_listener<0>([](std::unique_ptr<int>) {});

    EXPECT_THROW(yats::scheduler scheduler(pipeline), std::runtime_error);
}

