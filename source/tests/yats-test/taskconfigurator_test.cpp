#include <gmock/gmock.h>

#include <yats/slot.h>
#include <yats/task_configurator.h>

TEST(taskconfigurator_test, return_parameters)
{
    struct Task
    {
        yats::output_bundle<yats::slot<int, 0>> run(yats::slot<int, 0> input)
        {
            return std::make_tuple(static_cast<int>(input));
        }
    };

    yats::task_configurator<Task> configurator;

    configurator.input<0>();
    configurator.output<0>();
}

TEST(taskconfigurator_test, return_one_element_not_in_tuple)
{
    struct Task
    {
        yats::slot<int, 71> run(yats::slot<int, 14> input)
        {
            return input + 1;
        }
    };

    yats::task_configurator<Task> configurator;

    configurator.input<14>();
    configurator.output<71>();
}

TEST(taskconfigurator_test, multiple_returns_multiple_parameters)
{
    struct Task
    {
        yats::output_bundle<yats::slot<int, 0>, yats::slot<int, 1>> run(yats::slot<int, 0> input0, yats::slot<int, 1> input1)
        {
            return std::make_tuple(input0 + input1, input0 - input1);
        }
    };

    yats::task_configurator<Task> configurator;

    configurator.input<0>();
    configurator.output<0>();
    configurator.input<1>();
    configurator.output<1>();
}

TEST(taskconfigurator_test, get_input_output_by_id)
{
    struct Task
    {
        yats::output_bundle<yats::slot<int, 123>> run(yats::slot<int, 321> input)
        {
            return std::make_tuple(input + 1);
        }
    };

    yats::task_configurator<Task> configurator;
    configurator.input<321>();
    configurator.output<123>();
}

TEST(taskconfigurator_test, get_input_output_by_name)
{
    using namespace yats;
    struct Task
    {
        yats::output_bundle<yats::slot<int, "output"_id>> run(yats::slot<int, "input"_id> input)
        {
            return std::make_tuple(input + 1);
        }
    };

    yats::task_configurator<Task> configurator;
    configurator.input<"input"_id>();
    configurator.output<"output"_id>();
}

