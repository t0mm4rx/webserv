#ifndef REQUEST_INTERPRETOR_HPP
# define REQUEST_INTERPRETOR_HPP

# include <iostream>
# include <string>
# include <map>
# include <dirent.h>
# include "Configuration.hpp"
# include "parsing.hpp"

class RequestInterpretor
{
	private:
		std::string _request;
		std::string _ressource;
		Configuration::server _conf;
		Configuration::location _location;
		std::string _get(void);
		std::string _generateResponse(size_t code, std::map<std::string, std::string> headers, std::string content);
		std::string _generateResponse(size_t code, std::map<std::string, std::string> headers, const unsigned char *content, size_t content_size);
		std::string _getStatusDescription(size_t code);
		std::string _getErrorHTMLPage(size_t code);
		std::string _getListingHTMLPage(std::string path, std::string ressource);
		std::string _getMIMEType(std::string filename);
		Configuration::location _getLocation(std::string ressource);
	public:
		RequestInterpretor(std::string req = "", Configuration::server serverConf = Configuration::server());
		RequestInterpretor(const RequestInterpretor &other);
		RequestInterpretor &operator=(const RequestInterpretor &other);
		~RequestInterpretor(void);
		std::string getResponse(void);
};

# endif