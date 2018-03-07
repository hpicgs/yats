#include <gmock/gmock.h>

#include <yats/options.h>
#include <yats/scheduler.h>
#include <yats/slot.h>

using namespace yats;

struct task
{
    static options_map<task> options()
    {
        return {
            { "value", &task::val }
        };
    }

    slot<int, 0> run(slot<int, 0> input)
    {
        return input * val;
    }

private:
    int val = 5;
};

TEST(options_test, construct_with_options)
{
    // The task_configurator constructs the options object
    task_configurator<task> configurator;
}

TEST(options_test, construct_without_options)
{
    struct task_without_options
    {
        slot<int, 0> run(slot<int, 0> input)
        {
            return input * val;
        }

    private:
        int val = 5;
    };

    // The task_configurator constructs the options object
    task_configurator<task_without_options> configurator;
}

TEST(options_test, exception_invalid_key)
{
    task_configurator<task> configurator;
    auto options = configurator.options();

    EXPECT_ANY_THROW(options->update("new value", 1));
}

TEST(options_test, exception_invalid_type)
{
    task_configurator<task> configurator;
    auto options = configurator.options();

    EXPECT_ANY_THROW(options->update("value", 1.2));
    EXPECT_ANY_THROW(options->update("value", "1"));
    EXPECT_ANY_THROW(options->update("value", task()));
}

TEST(options_test, different_but_valid_type)
{
    task_configurator<task> configurator;
    auto options = configurator.options();

    const volatile int new_value = 12;
    EXPECT_NO_THROW(options->update("value", new_value));

    auto& reference_value = new_value;
    EXPECT_NO_THROW(options->update("value", reference_value));
}

TEST(options_test, runtime_update)
{
    struct runtime_task
    {
        runtime_task(int* external)
            : external(external)
        {
        }

        static options_map<runtime_task> options()
        {
            return {
                { "value", &runtime_task::val }
            };
        }

        void run()
        {
            *external = val;
        }

    private:
        int* external;
        int val = 5;
    };

    int local = 100;

    pipeline p;
    auto configurator = p.add<runtime_task>(&local);
    auto options = configurator->options();
    scheduler s(std::move(p));

    options->update("value", 200);
    EXPECT_EQ(local, 100);
    s.run();
    EXPECT_EQ(local, 200);
}

TEST(options_test, multiple_updates)
{
    struct runtime_task
    {
        runtime_task(int* external)
            : external(external)
        {
        }

        static options_map<runtime_task> options()
        {
            return {
                { "value", &runtime_task::val }
            };
        }

        void run()
        {
            *external = val;
        }

    private:
        int* external;
        int val = 5;
    };

    int local = 100;

    pipeline p;
    auto configurator = p.add<runtime_task>(&local);
    auto options = configurator->options();
    scheduler s(std::move(p));

    for (int i = 100; i >= 0; --i)
    {
        options->update("value", i);
    }

    EXPECT_EQ(local, 100);
    s.run();
    EXPECT_EQ(local, 0);
}
