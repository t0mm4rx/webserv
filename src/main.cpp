#include "Configuration.hpp"

int main(void)
{
	try
	{
		Configuration test = Configuration("./confs/test.conf");
		test.print();
	}
	catch (const ParsingException &e)
	{
		std::cerr << e.what() << std::endl;
	}

	return (0);
}