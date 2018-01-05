#include <parallel_sort.h>
#include <yats/Pipeline.h>

int main(int /*argc*/, char* /*argv*/[])
{
	yats::Pipeline pipeline;
	// TODO: split_task mit eigenen Werten initialisieren.
	auto split_task_configurator = pipeline.add<split_task>("split_task");
	auto sort_task_0_configurator = pipeline.add<sort_task>("sort_task_0");
	auto sort_task_1_configurator = pipeline.add<sort_task>("sort_task_1");
	auto join_task_configurator = pipeline.add<merge_task>("join_task");

	// HINWEIS: Alle Outputs m�ssen mit einem Input verbunden sein, da sonst eine
	// Exception fliegt.
	split_task_configurator->output("left"_id) >> sort_task_0_configurator->input("v"_id);
	split_task_configurator->output("right"_id) >> sort_task_1_configurator->input("v"_id);
	sort_task_0_configurator->output("sorted_vector"_id) >> join_task_configurator->input("left"_id);
	sort_task_1_configurator->output("sorted_vector"_id) >> join_task_configurator->input("right"_id);

	// FRAGE: Wie kann ich die sortierte Sequenz aus join_task auslesen?

	auto scheduler = pipeline.build();
	scheduler.run();
    return 0;
}
