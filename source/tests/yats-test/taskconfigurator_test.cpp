
#include <gmock/gmock.h>

#include <yats/Input.h>
#include <yats/Output.h>
#include <yats/TaskConfigurator.h>


template<typename T>
class OpenTaskConfigurator : public TaskConfigurator<T>
{
public:
	const std::map<size_t, InputProxy>& inputs() const { return m_inputs; }
	const std::map<size_t, OutputProxy>& outputs() const { return m_outputs; }
};


TEST(taskconfigurator_test, no_return_no_parameters)
{
	struct Task
	{
		void run()
		{

		}
	};

	OpenTaskConfigurator<Task> configurator;

	EXPECT_EQ(0, configurator.inputs().size());
	EXPECT_EQ(0, configurator.outputs().size());

	EXPECT_ANY_THROW(configurator.input(0));
	EXPECT_ANY_THROW(configurator.output(0));
}

TEST(taskconfigurator_test, no_return_parameters)
{
	struct Task
	{
		void run(Input<int, 0>)
		{

		}
	};

	OpenTaskConfigurator<Task> configurator;

	EXPECT_EQ(1, configurator.inputs().size());
	EXPECT_EQ(0, configurator.outputs().size());

	EXPECT_NO_THROW(configurator.input(0));
	EXPECT_ANY_THROW(configurator.output(0));
}

TEST(taskconfigurator_test, return_no_parameters)
{
	struct Task
	{
		OutputBundle<Output<int, 0>> run()
		{
			return 0;
		}
	};

	OpenTaskConfigurator<Task> configurator;

	EXPECT_EQ(0, configurator.inputs().size());
	EXPECT_EQ(1, configurator.outputs().size());

	EXPECT_ANY_THROW(configurator.input(0));
	EXPECT_NO_THROW(configurator.output(0));
}

TEST(taskconfigurator_test, return_parameters)
{
	struct Task
	{
		OutputBundle<Output<int, 0>> run(Input<int, 0> input)
		{
			return { static_cast<int>(input) };
		}
	};

	OpenTaskConfigurator<Task> configurator;

	EXPECT_EQ(1, configurator.inputs().size());
	EXPECT_EQ(1, configurator.outputs().size());

	EXPECT_NO_THROW(configurator.input(0));
	EXPECT_NO_THROW(configurator.output(0));
}

TEST(taskconfigurator_test, multiple_returns_multiple_parameters)
{
	struct Task
	{
		OutputBundle<Output<int, 0>, Output<int, 1>> run(Input<int, 0> input0, Input<int, 1> input1)
		{
			return { input0 + input1, input0 - input1 };
		}
	};

	OpenTaskConfigurator<Task> configurator;

	EXPECT_EQ(2, configurator.inputs().size());
	EXPECT_EQ(2, configurator.outputs().size());

	EXPECT_NO_THROW(configurator.input(0));
	EXPECT_NO_THROW(configurator.output(0));
	EXPECT_NO_THROW(configurator.input(1));
	EXPECT_NO_THROW(configurator.output(1));
}
