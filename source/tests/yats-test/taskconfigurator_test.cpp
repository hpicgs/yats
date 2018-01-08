#include <gmock/gmock.h>

#include <yats/input.h>
#include <yats/output.h>
#include <yats/task_configurator.h>

TEST(taskconfigurator_test, return_parameters)
{
    struct Task
    {
        yats::output_bundle<yats::output<int, 0>> run(yats::input<int, 0> input)
        {
            return std::make_tuple(static_cast<int>(input));
        }
    };

    yats::task_configurator<Task> configurator;

    EXPECT_NO_THROW(configurator.input<0>());
    EXPECT_NO_THROW(configurator.output<0>());
}

TEST(taskconfigurator_test, multiple_returns_multiple_parameters)
{
    struct Task
    {
        yats::output_bundle<yats::output<int, 0>, yats::output<int, 1>> run(yats::input<int, 0> input0, yats::input<int, 1> input1)
        {
            return std::make_tuple(input0 + input1, input0 - input1);
        }
    };

    yats::task_configurator<Task> configurator;

    EXPECT_NO_THROW(configurator.input<0>());
    EXPECT_NO_THROW(configurator.output<0>());
    EXPECT_NO_THROW(configurator.input<1>());
    EXPECT_NO_THROW(configurator.output<1>());
}

TEST(taskconfigurator_test, get_input_output_by_id)
{
    struct Task
    {
        yats::output_bundle<yats::output<int, 123>> run(yats::input<int, 321> input)
        {
            return std::make_tuple(input + 1);
        }
    };

    yats::task_configurator<Task> configurator;
    EXPECT_NO_THROW(configurator.input<321>());
    EXPECT_NO_THROW(configurator.output<123>());
}

TEST(taskconfigurator_test, get_input_output_by_name)
{
    using namespace yats;
    struct Task
    {
        yats::output_bundle<yats::output<int, "output"_id>> run(yats::input<int, "input"_id> input)
        {
            return std::make_tuple(input + 1);
        }
    };

    yats::task_configurator<Task> configurator;
    EXPECT_NO_THROW(configurator.input<"input"_id>());
    EXPECT_NO_THROW(configurator.output<"output"_id>());
}

