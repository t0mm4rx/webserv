#ifndef CONFIGURATION_HPP
# define CONFIGURATION_HPP

# include <vector>
# include <map>
# include <string>
# include <iostream>
# include "parsing.hpp"

class Configuration
{
	public:
		struct location
		{
			std::string name;
		};
		struct server
		{
			std::string name;
			std::string host;
			std::map<int, std::string> error_pages;
			std::vector<location> locations;
			size_t client_max_body_size;
			size_t port;
		};
	private:
		std::vector<server> _servers;
		void _parseConfig(std::string file);
		void _parseServer(std::string source, size_t line_start, size_t line_end);
	public:
		Configuration(void);
		Configuration(std::string file);
		Configuration &operator=(const Configuration &other);
		~Configuration(void);
		void print(void);
};

#endif