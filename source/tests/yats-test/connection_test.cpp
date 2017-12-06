
#include <gmock/gmock.h>


#include <yats/InputConnector.h>
#include <yats/OutputConnector.h>


TEST(connection_test, one_input_one_output)
{
	{
		InputConnector input;
		OutputConnector output;

		EXPECT_NO_THROW(input << output);
	}

	{
		InputConnector input;
		OutputConnector output;

		EXPECT_NO_THROW(output >> input);
	}
}

TEST(connection_test, one_input_multiple_output)
{
	InputConnector input;
	OutputConnector output1;
	OutputConnector output2;

	EXPECT_NO_THROW(input << output1);
	EXPECT_ANY_THROW(input << output2);
}

TEST(connection_test, multiple_input_one_output)
{
	InputConnector input1;
	InputConnector input2;
	OutputConnector output;

	EXPECT_NO_THROW(input1 << output);
	EXPECT_NO_THROW(input2 << output);
}
