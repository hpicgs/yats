#include <gmock/gmock.h>

#include <yats/scheduler.h>
#include <yats/slot.h>
#include <yats/thread_pool.h>
#include <yats/util.h>

#include <future>


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


class base
{
public:
    const std::string s = "mathias";
    virtual void do_something()
    {
        std::cout << s << std::endl;
    }
};

class derived : public base
{
public:
    const std::string s = "flueggen";
    void do_something() override
    {
        std::cout << s << std::endl;
    }
};

void foo(base & b)
{
    b.do_something();
}


class simple_task : public yats::abstract_task_container
{
public:
    simple_task() : abstract_task_container(std::set<size_t> {})
    {
    }

    ~simple_task()
    {
        std::cout << "~";
    }

    void run() override
    {
        for (size_t i = 0; i < 100000; i++)
        {

        }
    }

    bool can_run() const override
    {
        return  true;
    }
};

TEST(scheduler_test, playground)
{
   std::vector<std::thread> threads;

    auto start = std::chrono::high_resolution_clock::now();
 
    for (int i = 0; i<10000; i++)
    {
        threads.emplace_back(std::thread([]()
        {
            simple_task st;
            st.run();
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
    yats::thread_pool thread_pool(4);

    std::vector<std::unique_ptr<simple_task>> tasks;

    auto d = std::make_unique<derived>();
    foo(*d);

    
    tasks.reserve(10000);
    for (int i = 0; i<10000; i++)
    {
        tasks.push_back(std::make_unique<simple_task>());
        thread_pool.execute(tasks.back().get());
    }

    thread_pool.wait();
    const auto end = std::chrono::high_resolution_clock::now();

    std::cout << "Dauer: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << std::endl;
}

TEST(scheduler_test, playground_pooling_3)
{
    std::vector<std::future<void>> vec;
    vec.reserve(10010);
    const auto start = std::chrono::high_resolution_clock::now();
    

    for (int i = 0; i<10000; i++)
    {
        vec.emplace_back(std::async(std::launch::async,
            []() {
            for (size_t j = 0; j < 100000; j++)
            {

            }
        }));

    }
    
    for (auto &  element : vec) {
        element.get();
    }

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