#include <gmock/gmock.h>

#include <yats/scheduler.h>
#include <yats/slot.h>
#include <yats/thread_pool.h>
#include <yats/util.h>


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

    std::cout << "Dauer: " << duration << std::endl;

    EXPECT_GE(duration, 100);
    EXPECT_LE(duration, 500);
}

TEST(scheduler_test, throw_on_creation)
{
    yats::pipeline pipeline;

    auto source = pipeline.add([]() -> yats::slot<std::unique_ptr<int>, 0> { return std::make_unique<int>(0); });
    source->add_listener<0>([](std::unique_ptr<int>) {});
    source->add_listener<0>([](std::unique_ptr<int>) {});

    EXPECT_THROW(yats::scheduler scheduler(pipeline), std::runtime_error);
}

TEST(scheduler_test, playground)
{
    std::mutex mutex;
    std::vector<std::thread> threads;

    auto start = std::chrono::high_resolution_clock::now();
 
    for (int i = 0; i<10000; i++)
    {
        threads.emplace_back(std::thread([&mutex]()
        {
            //std::this_thread::sleep_for(std::chrono::milliseconds(400));
           // std::lock_guard<std::mutex> lock(mutex);
           // std::cout << "Running Thread " << i << std::endl;
            for (size_t i = 0; i < 100000; i++)
            {
                
            }
        }));
    }

    for (auto & thread : threads)
    {
        thread.join();
    }

    auto end = std::chrono::high_resolution_clock::now();

    std::cout << "Dauer: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << std::endl;
}

TEST(scheduler_test, playground_pooling_2)
{
    const auto start = std::chrono::high_resolution_clock::now();
    yats::thread_pool thread_pool(8);

    for (int i = 0; i<10000; i++)
    {
        thread_pool.execute([] {
            for (size_t j = 0; j < 100000; j++)
            {

            }
        });
    }

    thread_pool.wait();
    const auto end = std::chrono::high_resolution_clock::now();

    std::cout << "Dauer: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << std::endl;
}

TEST(scheduler_test, playground_pooling_1)
{
    yats::thread_pool pool(3);
    const auto start = std::chrono::high_resolution_clock::now();


    /*for (int i = 0; i < 1; i++)
    {
        pool.execute([] {
            std::cout << "Function 1: ";
            for (int i = 0; i < 100; i++)
            {
                std::cout << "." << " ";
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
            }
            std::cout << std::endl << "Finished" << std::endl;
        });

        pool.execute([] {
            std::cout << "Function 2: ";
            std::this_thread::sleep_for(std::chrono::milliseconds(400));
            for (int i = 0; i < 100; i++)
            {
                std::cout << "-" << " ";
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
            std::cout << std::endl << "Finished" << std::endl;
        });

        pool.execute([] {
            std::cout << "Function 3: ";
            std::this_thread::sleep_for(std::chrono::milliseconds(800));
            for (int i = 0; i < 100; i++)
            {
                std::cout << "*" << " ";
                std::this_thread::sleep_for(std::chrono::milliseconds(200));
            }
            std::cout << std::endl << "Finished" << std::endl;
        });

        pool.execute([] {
            std::cout << "Function 4: ";
            std::this_thread::sleep_for(std::chrono::milliseconds(1600));
            for (int i = 0; i < 100; i++)
            {
                std::cout << "/" << " ";
                std::this_thread::sleep_for(std::chrono::milliseconds(400));
            }
            std::cout << std::endl << "Finished" << std::endl;
        });

        pool.execute([] {
            std::cout << "Function 5: ";
            std::this_thread::sleep_for(std::chrono::milliseconds(3200));
            for (int i = 0; i < 100; i++)
            {
                std::cout << "+" << " ";
                std::this_thread::sleep_for(std::chrono::milliseconds(800));
            }
            std::cout << std::endl << "Finished" << std::endl;
        });
    }


    const auto end = std::chrono::high_resolution_clock::now();

    std::this_thread::sleep_for(std::chrono::seconds(3));

    std::cout << "Dauer: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << std::endl;*/
}