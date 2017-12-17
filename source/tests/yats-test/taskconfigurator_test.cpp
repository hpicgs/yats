#include <gmock/gmock.h>

#include <yats/Input.h>
#include <yats/Output.h>
#include <yats/TaskConfigurator.h>

/*

template<typename T>
class OpenTaskConfigurator : public yats::TaskConfigurator<T>
{
public:
	const std::map<uint64_t, std::unique_ptr<yats::AbstractInputConnector>>& inputs() const { return yats::TaskConfigurator<T>::m_inputs; }
	const std::map<uint64_t, std::unique_ptr<yats::AbstractOutputConnector>>& outputs() const { return yats::TaskConfigurator<T>::m_outputs; }
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
		void run(yats::Input<int, 0>)
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
		yats::OutputBundle<yats::Output<int, 0>> run()
		{
			return std::make_tuple(0);
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
		yats::OutputBundle<yats::Output<int, 0>> run(yats::Input<int, 0> input)
		{
			return std::make_tuple(static_cast<int>(input));
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
		yats::OutputBundle<yats::Output<int, 0>, yats::Output<int, 1>> run(yats::Input<int, 0> input0, yats::Input<int, 1> input1)
		{
			return std::make_tuple(input0 + input1, input0 - input1);
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

TEST(taskconfigurator_test, get_input_output_by_id)
{
	struct Task
	{
		yats::OutputBundle<yats::Output<int, 123>> run(yats::Input<int, 321> input)
		{
			return std::make_tuple(input + 1 );
		}
	};

	yats::TaskConfigurator<Task> configurator;
	EXPECT_NO_THROW(configurator.input(321));
	EXPECT_NO_THROW(configurator.output(123));

	EXPECT_ANY_THROW(configurator.input(0));
	EXPECT_ANY_THROW(configurator.output(0));
}

TEST(taskconfigurator_test, get_input_output_by_name)
{
	using namespace yats;
	struct Task
	{
		yats::OutputBundle<yats::Output<int, "output"_id>> run(yats::Input<int, "input"_id> input)
		{
			return std::make_tuple(input + 1);
		}
	};

	yats::TaskConfigurator<Task> configurator;
	EXPECT_NO_THROW(configurator.input("input"));
	EXPECT_NO_THROW(configurator.output("output"));

	EXPECT_ANY_THROW(configurator.input("output"));
	EXPECT_ANY_THROW(configurator.output("input"));
}

TEST(taskconfigurator_test, yo)
{
	std::map<std::string, std::unique_ptr<yats::AbstractTaskConfigurator>> yo;

	struct Task
	{
		void run()
		{
		}
	};

	yats::TaskConfigurator<Task>::build(yo);
}

*/
