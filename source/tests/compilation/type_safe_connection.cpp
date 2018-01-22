#include <iostream>
#include <string>

#include <yats/input_connector.h>
#include <yats/output_connector.h>
#include <yats/pipeline.h>

int main()
{
    yats::input_connector<int> input;
#ifdef SHOULD_FAIL
    yats::output_connector<std::string> output;
#else
    yats::output_connector<int> output;
#endif

    output >> input;

    return 0;
}
