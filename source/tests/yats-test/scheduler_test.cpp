#include <gmock/gmock.h>

#include <yats/scheduler.h>
#include <yats/slot.h>
#include <yats/util.h>

TEST(scheduler_test, simple_create)
{
    yats::pipeline empty_task_configs;
    EXPECT_NO_THROW(yats::scheduler{ empty_task_configs });
}

TEST(scheduler_test, multithreaded_timing_test)
{
    yats::pipeline pipeline;

    auto function = []() { std::this_thread::sleep_for(std::chrono::milliseconds(200)); };
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
