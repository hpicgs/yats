#include <iostream>
#include <string>

#include <yats/input_connector.h>
#include <yats/output_connector.h>
#include <yats/pipeline.h>

int main()
{
    yats::input_connector<int> input;
    yats::output_connector<std::string> output;

    output >> input;

	return 0;
}
