#include <gmock/gmock.h>

#include <yats/Pipeline.h>

class Source
{
public:
	int run()
	{
		std::cout << "Send 42" << std::endl;
		return 42;
	}
};

class Target
{
public:
	void run(int value)
	{
		std::cout << "Received " << value << std::endl;
	}
};


TEST(interface_creation, simple)
{
	yats::Pipeline pipeline;

	//auto sourceconfigurator = pipeline.add<Source>("ultra");
	//auto targetconfigurator = pipeline.add<Target>("ultimate");

	//sourceconfigurator->output() >> targetconfigurator->input();

	//pipeline.run();
}
