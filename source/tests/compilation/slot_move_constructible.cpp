#include <yats/slot.h>

using namespace yats;

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
    slot<test, 0> slot(t);
    return 0;
}
