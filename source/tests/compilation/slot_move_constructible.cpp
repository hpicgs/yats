#include <yats/slot.h>

struct test
{
    test() = default;
    test(const test&) = default;
#ifdef SHOULD_FAIL
    test(test&&) = delete;
#else
    test(test&&) = default;
#endif
};

int main()
{
    test t;
    yats::slot<test, 0> slot(t);
    return 0;
}
