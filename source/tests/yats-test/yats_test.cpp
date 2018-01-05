#include <gmock/gmock.h>

#include <yats/input.h>
#include <yats/output.h>
#include <yats/pipeline.h>

struct Source
{
    yats::output_bundle<yats::output<int, 0>> run()
    {
        std::cout << "Send 42" << std::endl;
        return std::make_tuple(42);
    }
};

struct Target
{
    void run(yats::input<int, 0> input)
    {
        std::cout << "Received " << input << ". Adding + 1" << std::endl;
    }
};

TEST(yats_test, simple)
{
    yats::pipeline pipeline;
    auto scheduler = pipeline.build();
    EXPECT_NO_THROW(scheduler.run());
}

TEST(yats_test, simple_connection)
{
    yats::pipeline pipeline;

    auto source_configurator = pipeline.add<Source>();
    auto target_configurator = pipeline.add<Target>();

    source_configurator->output<0>() >> target_configurator->input<0>();

    auto scheduler = pipeline.build();
    scheduler.run();
}