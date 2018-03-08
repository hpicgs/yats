#include <gmock/gmock.h>

#include <yats/slot.h>
#include <yats/scheduler.h>

using namespace yats;

struct Source
{
    output_bundle<slot<int, 0>> run()
    {
        return std::make_tuple(42);
    }
};

struct Target
{
    void run(slot<int, 0>)
    {
    }
};

TEST(yats_test, simple)
{
    pipeline pipeline;
    scheduler scheduler(std::move(pipeline));
    EXPECT_NO_THROW(scheduler.run());
}

TEST(yats_test, simple_connection)
{
    pipeline pipeline;

    auto source_configurator = pipeline.add<Source>();
    auto target_configurator = pipeline.add<Target>();

    source_configurator->output<0>() >> target_configurator->input<0>();

    scheduler scheduler(std::move(pipeline));
    scheduler.run();
}
