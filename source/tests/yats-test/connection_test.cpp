#include <gmock/gmock.h>


#include <yats/InputConnector.h>
#include <yats/OutputConnector.h>

TEST(connection_test, one_input_one_output)
{
	{
		yats::InputConnector input(nullptr);
		yats::OutputConnector output(nullptr);

		EXPECT_NO_THROW(input << output);
	}

	{
		yats::InputConnector input(nullptr);
		yats::OutputConnector output(nullptr);

		EXPECT_NO_THROW(output >> input);
	}
}

TEST(connection_test, one_input_multiple_output)
{
	yats::InputConnector input(nullptr);
	yats::OutputConnector output1(nullptr);
	yats::OutputConnector output2(nullptr);

	EXPECT_NO_THROW(input << output1);
	EXPECT_ANY_THROW(input << output2);
}

TEST(connection_test, multiple_input_one_output)
{
	yats::InputConnector input1(nullptr);
	yats::InputConnector input2(nullptr);
	yats::OutputConnector output(nullptr);

	EXPECT_NO_THROW(input1 << output);
	EXPECT_NO_THROW(input2 << output);
}
