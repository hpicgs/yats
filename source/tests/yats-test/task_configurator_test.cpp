#include <gmock/gmock.h>

#include <yats/slot.h>
#include <yats/task_configurator.h>

using namespace yats;

TEST(task_configurator_test, return_parameters)
{
    struct Task
    {
        output_bundle<slot<int, 0>> run(slot<int, 0> input)
        {
            return std::make_tuple(static_cast<int>(input));
        }
    };

    task_configurator<Task> configurator;

    configurator.input<0>();
    configurator.output<0>();
}

TEST(task_configurator_test, return_one_element_not_in_tuple)
{
    struct Task
    {
        slot<int, 71> run(slot<int, 14> input)
        {
            return input + 1;
        }
    };

    task_configurator<Task> configurator;

    configurator.input<14>();
    configurator.output<71>();
}

TEST(task_configurator_test, multiple_returns_multiple_parameters)
{
    struct Task
    {
        output_bundle<slot<int, 0>, slot<int, 1>> run(slot<int, 0> input0, slot<int, 1> input1)
        {
            return std::make_tuple(input0 + input1, input0 - input1);
        }
    };

    task_configurator<Task> configurator;

    configurator.input<0>();
    configurator.output<0>();
    configurator.input<1>();
    configurator.output<1>();
}

TEST(task_configurator_test, get_input_output_by_id)
{
    struct Task
    {
        output_bundle<slot<int, 123>> run(slot<int, 321> input)
        {
            return std::make_tuple(input + 1);
        }
    };

    task_configurator<Task> configurator;
    configurator.input<321>();
    configurator.output<123>();
}

TEST(task_configurator_test, get_input_output_by_name)
{
    struct Task
    {
        output_bundle<slot<int, "output"_id>> run(slot<int, "input"_id> input)
        {
            return std::make_tuple(input + 1);
        }
    };

    task_configurator<Task> configurator;
    configurator.input<"input"_id>();
    configurator.output<"output"_id>();
}

TEST(task_configurator_test, thread_constraints_default)
{
    struct Task
    {
        void run()
        {
        }
    };

    task_configurator<Task> configurator;
    auto constraints = configurator.thread_constraints();

    EXPECT_EQ(constraints.names().count(thread_group::any_thread_name()), 1);
    EXPECT_EQ(constraints.names().size(), 1);
}

TEST(task_configurator_test, thread_constraints_non_static_function)
{
    struct Task
    {
        thread_group thread_constraints()
        {
            return thread_group::main_thread();
        }

        void run()
        {
        }
    };

    task_configurator<Task> configurator;
    auto constraints = configurator.thread_constraints();

    EXPECT_EQ(constraints.names().count(thread_group::any_thread_name()), 1);
    EXPECT_EQ(constraints.names().size(), 1);
}

TEST(task_configurator_test, thread_constraints_static_function)
{
    struct Task
    {
        static thread_group thread_constraints()
        {
            return thread_group::main_thread();
        }

        void run()
        {
        }
    };

    task_configurator<Task> configurator;
    auto constraints = configurator.thread_constraints();

    EXPECT_EQ(constraints.names().count(thread_group::main_thread_name()), 1);
    EXPECT_EQ(constraints.names().size(), 1);
}
