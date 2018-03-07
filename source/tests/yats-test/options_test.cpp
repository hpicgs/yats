#include <gmock/gmock.h>

#include <yats/options.h>
#include <yats/scheduler.h>
#include <yats/slot.h>

using namespace yats;

TEST(options_test, options)
{
    struct Task
    {
        static options_map<Task> options()
        {
            return
            {
                { "value", &Task::val }
            };
        }

        output_bundle<slot<int, 0>> run(slot<int, 0> input)
        {
            return std::make_tuple(static_cast<int>(input * val));
        }

        int val = 5;
    };

    task_configurator<Task> configurator;
}
