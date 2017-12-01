
#include <gmock/gmock.h>


#include <yats/InputProxy.h>
#include <yats/OutputProxy.h>


TEST(connection_test, one_input_one_output)
{
	{
		InputProxy input;
		OutputProxy output;

		EXPECT_NO_THROW(input << output);
	}

	{
		InputProxy input;
		OutputProxy output;

		EXPECT_NO_THROW(output >> input);
	}
}

TEST(connection_test, one_input_multiple_output)
{
	InputProxy input;
	OutputProxy output1;
	OutputProxy output2;

	EXPECT_NO_THROW(input << output1);
	EXPECT_ANY_THROW(input << output2);
}

TEST(connection_test, multiple_input_one_output)
{
	InputProxy input1;
	InputProxy input2;
	OutputProxy output;

	EXPECT_NO_THROW(input1 << output);
	EXPECT_NO_THROW(input2 << output);
}
