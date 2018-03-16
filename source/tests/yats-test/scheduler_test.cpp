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

    auto function = []()
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    };

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

TEST(scheduler_test, run_twice)
{
    pipeline pipeline;

    int output = 0;
    auto task = pipeline.add([]() -> yats::slot<int, 0> { return 1; });
    task->add_listener<0>([&output](int value) {output += value; });

    yats::scheduler scheduler(std::move(pipeline));
    EXPECT_EQ(output, 0);
    scheduler.run();
    EXPECT_EQ(output, 1);
    scheduler.run();
    EXPECT_EQ(output, 2);
}

TEST(scheduler_test, throw_on_creation)
{
    pipeline pipeline;

    auto source = pipeline.add([]() -> slot<std::unique_ptr<int>, 0> { return std::make_unique<int>(0); });
    source->add_listener<0>([](std::unique_ptr<int>) {});
    source->add_listener<0>([](std::unique_ptr<int>) {});

    EXPECT_THROW(yats::scheduler scheduler(std::move(pipeline)), std::runtime_error);
}

TEST(scheduler_test, catch_task_exception)
{
    pipeline pipeline;

    struct task
    {
    	  void run()
        {
        	  throw std::runtime_error("failed foo");
        }
    };

    pipeline.add<task>();

    scheduler scheduler(std::move(pipeline));
    EXPECT_THROW(scheduler.run(), std::runtime_error);
}

TEST(scheduler_test, catch_chained_task_exception)
{
    pipeline pipeline;

    struct task
    {
        slot<int, 0> run()
        {
            throw std::runtime_error("failed foo");
            return 1;
        }
    };

    struct task2
    {
        void run(yats::slot<int, 0> input)
        {
            std::cout << *input + 1 << std::endl;
        }
    };

    auto first_task = pipeline.add<task>();
    auto second_task = pipeline.add<task2>();

    first_task->output<0>() >> second_task->input<0>();

    scheduler scheduler(std::move(pipeline));
    EXPECT_THROW(scheduler.run(), std::runtime_error);
}

TEST(scheduler_test, catch_last_task_exception)
{
    pipeline pipeline;

    struct task
    {
        slot<int, 0> run()
        {
            return 1;
        }
    };

    struct task2
    {
        void run(yats::slot<int, 0>)
        {
            throw std::runtime_error("failed foo");
        }
    };

    auto first_task = pipeline.add<task>();
    auto second_task = pipeline.add<task2>();

    first_task->output<0>() >> second_task->input<0>();

    scheduler scheduler(std::move(pipeline));
    EXPECT_THROW(scheduler.run(), std::runtime_error);
}

TEST(scheduler_test, catch_last_task_main_thread_exception)
{
    pipeline pipeline;

    struct task
    {
        slot<int, 0> run()
        {
            return 1;
        }
    };

    struct task2
    {
        thread_group thread_constraints()
        {
            return thread_group::main_thread();
        }

        void run(yats::slot<int, 0>)
        {
            throw std::runtime_error("failed foo");
        }
    };

    auto first_task = pipeline.add<task>();
    auto second_task = pipeline.add<task2>();

    first_task->output<0>() >> second_task->input<0>();

    scheduler scheduler(std::move(pipeline));
    EXPECT_THROW(scheduler.run(), std::runtime_error);
}

TEST(scheduler_test, catch_last_task_any_thread_exception)
{
    pipeline pipeline;

    struct task
    {
        slot<int, 0> run()
        {
            return 1;
        }
    };

    struct task2
    {
        thread_group thread_constraints()
        {
            return thread_group::any_thread();
        }

        void run(yats::slot<int, 0>)
        {
            throw std::runtime_error("failed foo");
        }
    };

    auto first_task = pipeline.add<task>();
    auto second_task = pipeline.add<task2>();

    first_task->output<0>() >> second_task->input<0>();

    scheduler scheduler(std::move(pipeline));
    EXPECT_THROW(scheduler.run(), std::runtime_error);
}

TEST(scheduler_test, catch_last_task_main_thread_after_any_thread_exception)
{
    pipeline pipeline;

    struct task
    {
        thread_group thread_constraints()
        {
            return thread_group::any_thread();
        }

        slot<int, 0> run()
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            throw std::runtime_error("failed foo");
            return 1;
        }
    };

    struct task2
    {
        thread_group thread_constraints()
        {
            return thread_group::main_thread();
        }

        void run(yats::slot<int, 0> input)
        {
            std::cout << *input + 1 << std::endl;
        }
    };

    auto first_task = pipeline.add<task>();
    auto second_task = pipeline.add<task2>();

    first_task->output<0>() >> second_task->input<0>();

    scheduler scheduler(std::move(pipeline));
    EXPECT_THROW(scheduler.run(), std::runtime_error);
}
