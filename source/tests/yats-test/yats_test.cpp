#include <array>
#include <cstdlib>

#include <gmock/gmock.h>

#include <yats/Input.h>
#include <yats/Output.h>
#include <../../tests/yats-test/parallel_sort.h>
#include <yats/Pipeline.h>

struct Source
{
	yats::OutputBundle<yats::Output<int, 0>> run()
	{
		std::cout << "Send 42" << std::endl;
		return std::make_tuple(42);
	}
};

struct Target
{
	void run(yats::Input<int, 0> input)
	{
		std::cout << "Received " << input << ". Adding + 1" << std::endl;
	}
};


TEST(yats_test, simple)
{
	yats::Pipeline pipeline;
	auto scheduler = pipeline.build();
	EXPECT_NO_THROW(scheduler.run());
}

TEST(yats_test, simple_connection)
{
	yats::Pipeline pipeline;

	auto source_configurator = pipeline.add<Source>("source");
	auto target_configurator = pipeline.add<Target>("target");

	source_configurator->output(0) >> target_configurator->input(0);

	auto scheduler = pipeline.build();
	scheduler.run();
}

TEST(parallel_sort, split_task)
{
	yats::Pipeline pipeline;
	// FRAGE: Ich möchte split_task gerne mit eigenen Werten initialisieren.
	auto split_task_configurator = pipeline.add<yats::parallel_sort::split_task>("split_task");
	auto sort_task_0_configurator = pipeline.add<yats::parallel_sort::sort_task>("sort_task_0");
	auto sort_task_1_configurator = pipeline.add<yats::parallel_sort::sort_task>("sort_task_1");
	auto join_task_configurator = pipeline.add<yats::parallel_sort::join_task>("join_task");

	// HINWEIS: Alle Outputs müssen mit einem Input verbunden sein, da sonst eine
	// Exception fliegt.
	split_task_configurator->output(0) >> sort_task_0_configurator->input(0);
	split_task_configurator->output(1) >> sort_task_1_configurator->input(0);
	sort_task_0_configurator->output(0) >> join_task_configurator->input(0);
	sort_task_1_configurator->output(0) >> join_task_configurator->input(1);

	// FRAGE: Wie kann ich die sortierte Sequenz aus join_task auslesen?

	auto scheduler = pipeline.build();
	scheduler.run();
}