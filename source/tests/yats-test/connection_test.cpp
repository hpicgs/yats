#include <gmock/gmock.h>

#include <yats/input_connector.h>
#include <yats/output_connector.h>

TEST(connection_test, one_input_one_output)
{
    {
        yats::input_connector<size_t> input;
        yats::output_connector<size_t> output;

        EXPECT_NO_THROW(input << output);
    }

    {
        yats::input_connector<size_t> input;
        yats::output_connector<size_t> output;

        EXPECT_NO_THROW(output >> input);
    }
}

TEST(connection_test, one_input_multiple_output)
{
    yats::input_connector<size_t> input;
    yats::output_connector<size_t> output1;
    yats::output_connector<size_t> output2;

    EXPECT_NO_THROW(input << output1);
    EXPECT_ANY_THROW(input << output2);
}

TEST(connection_test, multiple_input_one_output)
{
    yats::input_connector<size_t> input1;
    yats::input_connector<size_t> input2;
    yats::output_connector<size_t> output;

    EXPECT_NO_THROW(input1 << output);
    EXPECT_NO_THROW(input2 << output);
}
