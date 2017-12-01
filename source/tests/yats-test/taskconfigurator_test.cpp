
#include <gmock/gmock.h>

#include <yats/TaskConfigurator.h>


template<typename T>
class OpenTaskConfigurator : public TaskConfigurator<T>
{
public:
	const std::map<size_t, InputProxy>& inputs() const { return m_inputs; }
	const std::map<size_t, OutputProxy>& outputs() const { return m_outputs; }
};


TEST(taskconfigurator_test, task_no_return_no_parameters)
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

TEST(taskconfigurator_test, task_no_return)
{
	struct Task
	{
		void run(Input<int, 0> input)
		{

		}
	};

	OpenTaskConfigurator<Task> configurator;

	EXPECT_EQ(0, configurator.inputs().size());
	EXPECT_EQ(0, configurator.outputs().size());

	EXPECT_ANY_THROW(configurator.input(0));
	EXPECT_ANY_THROW(configurator.output(0));
}
