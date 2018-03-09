#include <gmock/gmock.h>

#include <yats/class_name.h>

using namespace yats;

TEST(class_name_test, untemplated_class_test)
{
    EXPECT_EQ(class_name::get<int>(), "int");
    EXPECT_EQ(class_name::get<float>(), "float");
    EXPECT_EQ(class_name::get<char>(), "char");
    EXPECT_EQ(class_name::get<const int>(), "const int");
}

TEST(class_name_test, nested_class_test)
{
    struct some_struct
    {
    };

    class some_class
    {
    };

    EXPECT_EQ(class_name::get<some_struct>(), "some_struct");
    EXPECT_EQ(class_name::get<some_class>(), "some_class");
}

template <typename T, class C>
class some_templated_class
{
};

TEST(class_name_test, templated_nested_class_test)
{
    using why = some_templated_class<char, int>;
    EXPECT_EQ(class_name::get<why>(), "some_templated_class<char,int>");

    /*std::cout <<  msvc_class_name_parser::get_gcc_class_name<int>("std::string get() [with T = YOLO; std::string = std::basic_string<char>]") << std::endl;
    std::cout << msvc_class_name_parser::get_gcc_class_name<int>("std::string get()[with T = int; std::string = std::basic_string<char>]") << std::endl;
    std::cout << msvc_class_name_parser::get_gcc_class_name<int>("std::string get()[with T = main()::some_struct; std::string = std::basic_string<char>]") << std::endl;
    std::cout << msvc_class_name_parser::get_gcc_class_name<int>("std::string get()[with T = YOLO2<main()::some_struct>; std::string = std::basic_string<char>]") << std::endl;
    std::cout << msvc_class_name_parser::get_gcc_class_name<int>("std::string get()[with T = YOLO2<main()::some_struct, int>; std::string = std::basic_string<char>]") << std::endl;*/
    
}
