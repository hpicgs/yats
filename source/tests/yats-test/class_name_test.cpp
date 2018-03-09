#include <gmock/gmock.h>
#include <map>

#include <yats/class_name.h>

using namespace yats;

TEST(class_name_test, untemplated_class_test)
{
    EXPECT_EQ(get_class_name<int>(), "int");
    EXPECT_EQ(get_class_name<float>(), "float");
    EXPECT_EQ(get_class_name<char*>(), "char*");
}

TEST(class_name_test, nested_class_test)
{
    struct some_struct
    {
    };

    class some_class
    {
    };

    EXPECT_EQ(get_class_name<some_struct>(), "some_struct");
    EXPECT_EQ(get_class_name<some_class>(), "some_class");
    EXPECT_EQ(get_class_name<some_class*>(), "some_class*");
}

template <typename T, class C>
class some_templated_class
{
};

TEST(class_name_test, templated_nested_class_test)
{
    using why = some_templated_class<char*, int>;
    EXPECT_EQ(get_class_name<why>(), "some_templated_class<char *,int>");
}
