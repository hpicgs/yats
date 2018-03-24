#include <gmock/gmock.h>

#include <yats/pipeline.h>
#include <yats/scheduler.h>
#include <yats/slot.h>

#include <algorithm>

using namespace yats;

TEST(pipeline_test, add_lambda_task)
{
    pipeline pipeline;

    auto test_int = 25;
    auto lambda_source = pipeline.add([]() -> slot<int, 1> { return 30; });
    auto lambda_target = pipeline.add([&test_int](slot<int, 1> value) mutable { test_int = value; });

    lambda_source->output<1>() >> lambda_target->input<1>();

    scheduler scheduler(std::move(pipeline));
    scheduler.run();

    EXPECT_EQ(test_int, 30);
}

TEST(pipeline_test, add_one_listener)
{
    pipeline pipeline;

    auto lambda_source = pipeline.add([]() -> slot<int, 0> { return 30; });

    auto output = 0;
    lambda_source->add_listener<0>([&output](int value) { output = value; });

    scheduler scheduler(std::move(pipeline));
    scheduler.run();

    EXPECT_EQ(output, 30);
}

TEST(pipeline_test, add_listener_and_task)
{
    pipeline pipeline;

    auto lambda_source = pipeline.add([]() -> slot<int, 0> { return 30; });
    auto lambda_target = pipeline.add([](slot<int, 0>) {});

    auto output = 0;
    lambda_source->add_listener<0>([&output](int value) { output = value; });

    lambda_source->output<0>() >> lambda_target->input<0>();

    scheduler scheduler(std::move(pipeline));
    scheduler.run();

    EXPECT_EQ(output, 30);
}

TEST(pipeline_test, add_no_listener)
{
    pipeline pipeline;

    auto lambda_source = pipeline.add([]() -> slot<int, 0> { return 30; });
    (void) lambda_source;

    scheduler scheduler(std::move(pipeline));
    scheduler.run();
}

TEST(pipeline_test, add_multiple_listener)
{
    pipeline pipeline;

    auto lambda_source = pipeline.add([]() -> slot<int, 0> { return 30; });

    auto output1 = 0;
    auto output2 = 0;
    lambda_source->add_listener<0>([&output1](int value) { output1 = value; });
    lambda_source->add_listener<0>([&output2](int value) { output2 = value; });

    scheduler scheduler(std::move(pipeline));
    scheduler.run();

    EXPECT_EQ(output1, 30);
    EXPECT_EQ(output2, 30);
}

TEST(pipeline_test, use_external_input)
{
    pipeline pipeline;
    auto expected_value = 0;

    auto lambda_target = pipeline.add([&expected_value](slot<int, 0> input) { expected_value = input; });
    auto writer = lambda_target->mark_as_external<0>();

    scheduler scheduler(std::move(pipeline));

    writer(15);
    scheduler.run();
    EXPECT_EQ(expected_value, 15);

    writer(30);
    scheduler.run();
    EXPECT_EQ(expected_value, 30);
}

TEST(pipeline_test, external_input_by_move)
{
    pipeline pipeline;
    auto expected_value = 0;

    auto lambda_target = pipeline.add([&expected_value](slot<std::unique_ptr<int>, 0> input) { expected_value = **input; });
    auto writer = lambda_target->mark_as_external<0>();

    scheduler scheduler(std::move(pipeline));
    writer(std::make_unique<int>(15));

    scheduler.run();
    EXPECT_EQ(expected_value, 15);
}

template <typename T, class C>
class some_templated_class
{
public:
    void run()
    {
    }
};

TEST(pipeline_test, save_to_file)
{
    pipeline p;

    struct add_task
    {
        slot<int, "sum"_id> run(slot<int, "summand_a"_id> a, slot<int, "summand_b"_id> b)
        {
            return a + b;
        }
    };

    struct multiply_task
    {
        slot<int, "product"_id> run(slot<int, "factor_a"_id> a, slot<int, "factor_b"_id> b)
        {
            return a * b;
        }
    };

    auto add_cfg = p.add<add_task>();
    auto multiply_cfg = p.add<multiply_task>();
    p.add<some_templated_class<int, int>>();

    add_cfg->output<"sum"_id>() >> multiply_cfg->input<"factor_a"_id>();

    p.save_to_file("graph.txt");

    std::string b = "<Hallo>";
    std::string str;
    for (const auto& c : b)
    {
        switch(c)
        {
        case '<':
            str.append("&lt;");
            break;
        case '>':
            str.append("&gt;");
            break;
        default:
            str.append(1, c);
        }
    }
    std::cout << str << std::endl;
}
