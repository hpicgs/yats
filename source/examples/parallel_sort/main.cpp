#include <parallel_sort.h>
#include <yats/Pipeline.h>

int main(int /*argc*/, char* /*argv*/[])
{
	yats::Pipeline pipeline;
	// FRAGE: Ich möchte split_task gerne mit eigenen Werten initialisieren.
	auto split_task_configurator = pipeline.add<split_task>("split_task");
	auto sort_task_0_configurator = pipeline.add<sort_task>("sort_task_0");
	auto sort_task_1_configurator = pipeline.add<sort_task>("sort_task_1");
	auto join_task_configurator = pipeline.add<join_task>("join_task");

	// HINWEIS: Alle Outputs müssen mit einem Input verbunden sein, da sonst eine
	// Exception fliegt.
	split_task_configurator->output(0) >> sort_task_0_configurator->input(0);
	split_task_configurator->output(1) >> sort_task_1_configurator->input(0);
	sort_task_0_configurator->output(0) >> join_task_configurator->input(0);
	sort_task_1_configurator->output(0) >> join_task_configurator->input(1);

	// FRAGE: Wie kann ich die sortierte Sequenz aus join_task auslesen?

	auto scheduler = pipeline.build();
	scheduler.run();
    return 0;
}
