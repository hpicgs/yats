#include <gmock/gmock.h>


#include <yats/InputConnector.h>
#include <yats/OutputConnector.h>

namespace yats
{

TEST(connection_test, one_input_one_output)
{
	{
		InputConnector input(nullptr);
		OutputConnector output(nullptr);

		EXPECT_NO_THROW(input << output);
	}

	{
		InputConnector input(nullptr);
		OutputConnector output(nullptr);

		EXPECT_NO_THROW(output >> input);
	}
}

TEST(connection_test, one_input_multiple_output)
{
	InputConnector input(nullptr);
	OutputConnector output1(nullptr);
	OutputConnector output2(nullptr);

	EXPECT_NO_THROW(input << output1);
	EXPECT_ANY_THROW(input << output2);
}

TEST(connection_test, multiple_input_one_output)
{
	InputConnector input1(nullptr);
	InputConnector input2(nullptr);
	OutputConnector output(nullptr);

	EXPECT_NO_THROW(input1 << output);
	EXPECT_NO_THROW(input2 << output);
}

} // namespace yats
