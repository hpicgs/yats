#include <iostream>
#include <yats/pipeline.h>

int main()
{
#ifdef SHOULD_FAIL
    std::cout < "Hello World";
#else
    std::cout << "Hello World";
#endif

    return 0;
}
