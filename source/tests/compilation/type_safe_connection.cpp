#include <string>

#include <yats/input_connector.h>
#include <yats/output_connector.h>

using namespace yats;

int main()
{
    input_connector<int> input;
#ifdef SHOULD_FAIL
    output_connector<std::string> output;
#else
    output_connector<int> output;
#endif

    output >> input;

    return 0;
}
