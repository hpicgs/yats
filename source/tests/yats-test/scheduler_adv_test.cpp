#include <gmock/gmock.h>

#include <yats/scheduler_adv.h>
#include <yats/slot.h>
#include <yats/util.h>

/**
 * Create a set of tasks with different durations. These tasks are used to test the
 * Scheduler.
 */

class sleep_task
{
public:
    sleep_task(const size_t duration_in_ms) : m_duration_in_ms(duration_in_ms)
    {
    }

    void run()
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(m_duration_in_ms));
    }

protected:
    size_t m_duration_in_ms;
};





TEST(scheduler_adv_test, empty_pipeline)
{
    const yats::pipeline empty_pipeline;
    EXPECT_NO_THROW(yats::scheduler_adv{ empty_pipeline });
}

TEST(scheduler_adv_test, experiment_with_time_measurements)
{
    yats::pipeline pipeline;
    pipeline.add<sleep_task>(200);

    yats::scheduler_adv scheduler(pipeline);

    auto start = std::chrono::high_resolution_clock::now();
    for (auto i = 0; i < 5; i++)
    {
       // scheduler.run();
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

  //  std::cout << "Hallo" << std::endl;

  //  std::cout << "Ausführungszeit" << duration << std::endl;
}

TEST(scheduler_adv_test, multithreaded_timing_test)
{
    yats::pipeline pipeline;

    auto function = []() { std::this_thread::sleep_for(std::chrono::milliseconds(200)); };
    pipeline.add(function);
    pipeline.add(function);
    pipeline.add(function);

    yats::scheduler_adv scheduler(pipeline);
    auto start = std::chrono::high_resolution_clock::now();
    scheduler.run();
    auto end = std::chrono::high_resolution_clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    EXPECT_GE(duration, 100);
    EXPECT_LE(duration, 500);
}