#include <gmock/gmock.h>

#include <yats/scheduler.h>
#include <yats/slot.h>

using namespace yats;

TEST(scheduler_test, simple_create)
{
    EXPECT_NO_THROW(yats::scheduler{ yats::pipeline() });
}

TEST(scheduler_test, multithreaded_timing_test)
{
    pipeline pipeline;

    auto function = []() { std::this_thread::sleep_for(std::chrono::milliseconds(200)); };
    pipeline.add(function);
    pipeline.add(function);
    pipeline.add(function);

    scheduler scheduler(std::move(pipeline));
    auto start = std::chrono::high_resolution_clock::now();
    scheduler.run();
    auto end = std::chrono::high_resolution_clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    EXPECT_GE(duration, 100);
    EXPECT_LE(duration, 500);
}

TEST(scheduler_test, throw_on_creation)
{
    pipeline pipeline;

    auto source = pipeline.add([]() -> slot<std::unique_ptr<int>, 0> { return std::make_unique<int>(0); });
    source->add_listener<0>([](std::unique_ptr<int>) {});
    source->add_listener<0>([](std::unique_ptr<int>) {});

    EXPECT_THROW(yats::scheduler scheduler(std::move(pipeline)), std::runtime_error);
}
