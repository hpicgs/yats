#include <test_util.h>

#include <gmock/gmock.h>

#include <yats/scheduler.h>
#include <yats/slot.h>

using namespace yats;

TEST(custom_constructor_test, reference_as_argument)
{
    struct Source
    {
        explicit Source(const int& value)
            : m_value(value)
        {
        }

        output_bundle<slot<int, 0>> run()
        {
            return { m_value };
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

        void run(slot<int, 0> value)
        {
            m_value = value;
        }

    private:
        int& m_value;
    };

    int start_value = 1;
    int end_value = -1;

    pipeline pipeline;
    auto source = pipeline.add<Source>(std::cref(start_value));
    auto target = pipeline.add<Target>(std::ref(end_value));

    source->output<0>() >> target->input<0>();

    scheduler scheduler(std::move(pipeline));

    EXPECT_NE(start_value, end_value);
    start_value = 2;
    scheduler.run();
    EXPECT_EQ(start_value, end_value);
}

TEST(custom_constructor_test, no_reference_as_argument)
{
    struct Task
    {
        Task(uint8_t no_reference, uint8_t& reference)
            : m_no_reference(no_reference)
            , m_reference(reference)
        {
        }

        void run()
        {
            m_reference = m_no_reference;
        }

        uint8_t m_no_reference;
        uint8_t& m_reference;
    };

    pipeline pipeline;

    uint8_t output_value = 10;
    uint8_t input_value = 0;
    pipeline.add<Task>(input_value, std::ref(output_value));
    input_value += 5;

    scheduler scheduler(std::move(pipeline));
    scheduler.run();

    EXPECT_EQ(output_value, 0);
}

TEST(custom_constructor_test, no_copy)
{
    struct Task
    {
        Task(uint8_t& c, constructor_counter cc)
            : copied(c)
            , counter(std::move(cc))
        {
        }

        void run()
        {
            copied = counter.copied;
        }

        uint8_t& copied;
        constructor_counter counter;
    };

    pipeline pipeline;

    uint8_t copy_counter = 0;
    pipeline.add<Task>(std::ref(copy_counter), constructor_counter());

    scheduler scheduler(std::move(pipeline));
    scheduler.run();

    EXPECT_EQ(copy_counter, 0);
}
