#include <gmock/gmock.h>

#include <yats/input_connector.h>
#include <yats/output_connector.h>

using namespace yats;

TEST(connection_test, one_input_one_output)
{
    {
        input_connector<size_t> input;
        output_connector<size_t> output;

        EXPECT_NO_THROW(input << output);
    }

    {
        input_connector<size_t> input;
        output_connector<size_t> output;

        EXPECT_NO_THROW(output >> input);
    }
}

TEST(connection_test, one_input_multiple_output)
{
    input_connector<size_t> input;
    output_connector<size_t> output1;
    output_connector<size_t> output2;

    EXPECT_NO_THROW(input << output1);
    EXPECT_ANY_THROW(input << output2);
}

TEST(connection_test, multiple_input_one_output)
{
    input_connector<size_t> input1;
    input_connector<size_t> input2;
    output_connector<size_t> output;

    EXPECT_NO_THROW(input1 << output);
    EXPECT_NO_THROW(input2 << output);
}
