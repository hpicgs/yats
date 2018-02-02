#include <gmock/gmock.h>

#include <yats/slot.h>
#include <yats/scheduler.h>

struct Source
{
    yats::output_bundle<yats::slot<int, 0>> run()
    {
        std::cout << "Send 42" << std::endl;
        return std::make_tuple(42);
    }
};

struct Target
{
    void run(yats::slot<int, 0> input)
    {
        std::cout << "Received " << input << ". Adding + 1" << std::endl;
    }
};

TEST(yats_test, simple)
{
    yats::pipeline pipeline;
    yats::scheduler scheduler(pipeline);
    EXPECT_NO_THROW(scheduler.run());
}

TEST(yats_test, simple_connection)
{
    yats::pipeline pipeline;

    auto source_configurator = pipeline.add<Source>();
    auto target_configurator = pipeline.add<Target>();

    source_configurator->output<0>() >> target_configurator->input<0>();

    yats::scheduler scheduler(pipeline);
    scheduler.run();
}