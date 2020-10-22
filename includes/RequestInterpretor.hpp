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
		std::string _get(void);
		std::string _generateResponse(size_t code, std::map<std::string, std::string> headers, std::string content);
		std::string _getStatusDescription(size_t code);
		std::string _getErrorHTMLPage(size_t code);
		std::string _getListingHTMLPage(std::string path, std::string ressource);
		std::string _getMIMEType(std::string filename);
	public:
		RequestInterpretor(std::string req = "", Configuration::server serverConf = Configuration::server());
		RequestInterpretor(const RequestInterpretor &other);
		RequestInterpretor &operator=(const RequestInterpretor &other);
		~RequestInterpretor(void);
		std::string getResponse(void);
};

# endif