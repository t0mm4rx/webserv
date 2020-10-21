#include "Configuration.hpp"

int main(void)
{
	try
	{
		Configuration test = Configuration("./confs/conf-1.conf");
		test.print();
		std::cout << test.getServers().size() << std::endl;
	}
	catch (const ParsingException &e)
	{
		std::cerr << e.what() << std::endl;
	}

	return (0);
}