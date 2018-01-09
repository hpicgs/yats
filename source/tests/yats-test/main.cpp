#include <gmock/gmock.h>

int main(int argc, char* argv[])
{
    ::testing::InitGoogleMock(&argc, argv);
    int yolo = 5;
    return RUN_ALL_TESTS();
}
