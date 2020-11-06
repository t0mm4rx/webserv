#ifndef CONFIGURATION_HPP
# define CONFIGURATION_HPP

# include <vector>
# include <map>
# include <string>
# include <iostream>
# include <sstream>
# include "parsing.hpp"

class Configuration
{
	public:
		struct location
		{
			std::string name;
			std::string root;
			std::vector<std::string> methods;
			bool autoindex;
			std::string index;
			std::vector<std::string> cgi_extension;
			std::string cgi_path;
			bool upload_enable;
			std::string upload_path;
			size_t client_max_body_size;
		};
		struct server
		{
			std::vector<std::string> names;
			std::string host;
			std::string root;
			std::map<int, std::string> error_pages;
			std::vector<location> locations;
			size_t port;
		};
	private:
		std::vector<server> _servers;
		void _parseConfig(std::string file);
		void _parseServer(std::string source, size_t line_start, size_t line_end);
		void _parseServerProperty(std::string source, size_t line, server &s);
		void _parseLocationProperty(std::string source, size_t line, location &l);
		void _validateConfig(void);
		void _completeConfig(void);
		server _defaultServer(void);
		location _defaultLocation(void);
		location _parseLocation(std::string source, size_t line_start, size_t line_end);
	public:
		Configuration(void);
		Configuration(std::string file);
		Configuration &operator=(const Configuration &other);
		~Configuration(void);
		std::vector<server> getServers(void);
		void print(void);
};

#endif