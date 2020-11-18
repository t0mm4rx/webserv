#ifndef REQUEST_INTERPRETOR_HPP
# define REQUEST_INTERPRETOR_HPP

# include <iostream>
# include <string>
# include <map>
# include <dirent.h>
# include <time.h>
# include <algorithm>
# include <sys/time.h>
# include <stdio.h>
# include "Configuration.hpp"
# include "parsing.hpp"
# include "WebServ.hpp"
# include "HeadersBlock.hpp"
# include "CGI.hpp"

class RequestInterpretor
{
	private:
		std::string _ressource;
		HeadersBlock _header_block;
		Configuration::server _conf;
		Configuration::location _location;
		std::string _get(std::string ressource_path, std::map<std::string, std::string> headers, bool send_body=true);
		std::string _head(std::string ressource_path, std::map<std::string, std::string> headers);
		std::string _post(std::string ressource_path, std::map<std::string, std::string> headers);
		std::string _put(std::string ressource_path, std::map<std::string, std::string> headers);
		std::string _delete(std::string ressource_path, std::map<std::string, std::string> headers);
		std::string _trace(std::map<std::string, std::string> headers);
		std::string _options(std::map<std::string, std::string> headers);
		std::string _wrongMethod(void);
		std::string _generateResponse(size_t code, std::map<std::string, std::string> headers, std::string content);
		std::string _generateResponse(size_t code, std::map<std::string, std::string> headers, const unsigned char *content, size_t content_size);
		std::string _getStatusDescription(size_t code);
		std::string _getErrorHTMLPage(size_t code);
		std::string _getListingHTMLPage(std::string path, std::string ressource);
		std::string _getMIMEType(std::string filename);
		std::string _getDateHeader(void);
		std::string _formatTimestamp(time_t timestamp);
		Configuration::location _getLocation(std::string ressource);
		bool _isMethodAllowed(std::string method);
		std::string _formatRessource(std::string ressource);
		bool _shouldCallCGI(std::string ressource_path);
		std::string _addCGIHeaders(std::string response);
		std::string _getCGIStatus(std::string response);
	public:
		RequestInterpretor(HeadersBlock &header_block, Configuration::server serverConf = Configuration::server());
		RequestInterpretor(const RequestInterpretor &other);
		RequestInterpretor &operator=(const RequestInterpretor &other);
		~RequestInterpretor(void);
		std::string getResponse(void);
};

# endif