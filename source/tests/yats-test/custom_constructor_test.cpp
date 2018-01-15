#include <gmock/gmock.h>

#include <yats/slot.h>
#include <yats/pipeline.h>
#include <yats/scheduler.h>

TEST(custom_constructor_test, reference_as_argument)
{
    struct Source
    {
        explicit Source(const int& value)
            : m_value(value)
        {
        }

        yats::output_bundle<yats::slot<int, 0>> run()
        {
            return std::make_tuple(m_value);
        }

    private:
        const int& m_value;
    };

    struct Target
    {
        explicit Target(int& value)
            : m_value(value)
        {
        }

        void run(yats::slot<int, 0> value)
        {
            m_value = value;
        }

    private:
        int& m_value;
    };

    int start_value = 1;
    int end_value = -1;

    yats::pipeline pipeline;
    auto source = pipeline.add<Source>(start_value);
    auto target = pipeline.add<Target>(end_value);

    source->output<0>() >> target->input<0>();
    
    auto scheduler = yats::scheduler(pipeline);

    EXPECT_NE(start_value, end_value);
    start_value = 2;
    scheduler.run();
    EXPECT_EQ(start_value, end_value);
}
