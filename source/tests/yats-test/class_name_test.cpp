#include <gmock/gmock.h>

#include <yats/class_name.h>

using namespace yats;

class some_class
{
};

TEST(class_name_test, untemplated_class_test)
{
    EXPECT_EQ(class_name::get<int>(), "int");
    EXPECT_EQ(class_name::get<float>(), "float");
    EXPECT_EQ(class_name::get<char>(), "char");
    EXPECT_EQ(class_name::get<const int>(), "int");
    EXPECT_EQ(class_name::get<some_class>(), "some_class");
}

TEST(class_name_test, nested_class_test)
{
    struct some_struct
    {
    };

    class some_nested_class
    {
    };

    EXPECT_EQ(class_name::get<some_struct>(), "some_struct");
    EXPECT_EQ(class_name::get<some_nested_class>(), "some_nested_class");
}

template <typename T, class C>
class some_templated_class
{
};

TEST(class_name_test, templated_nested_class_test)
{
    using why = some_templated_class<char, int>;
    EXPECT_EQ(class_name::get<why>(), "some_templated_class&lt;char,int&gt;");
}
