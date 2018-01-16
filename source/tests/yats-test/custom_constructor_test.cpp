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

struct constructor_counter
{
    constructor_counter() = default;
    constructor_counter(const constructor_counter& rhs)
    {
        copied = rhs.copied + 1;
        moved = rhs.moved;
    }

    constructor_counter(constructor_counter&& rhs)
    {
        copied = rhs.copied;
        moved = rhs.moved + 1;
    }

    uint8_t copied = 0;
    uint8_t moved = 0;
};

TEST(custom_constructor_test, no_unnecessary_copy)
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

    yats::pipeline pipeline;

    uint8_t copy_counter = 0;
    pipeline.add<Task>(copy_counter, constructor_counter());

    // This should be the only place, where the copy constructor is called
    // Also it should be called only once
    yats::scheduler scheduler(pipeline);
    scheduler.run();

    EXPECT_EQ(copy_counter, 1);
}
