#ifndef CGI_CPP
# define CGI_CPP

# include <iostream>
# include <string>
# include <map>
# include <stdlib.h>
# include "HeadersBlock.hpp"
# include "Configuration.hpp"

class CGI
{
	private:
		std::string _cgi_path;
		std::string _file;
		HeadersBlock _request;
		Configuration::server _conf;
		std::map<std::string, std::string> _getParams(void);
		std::string _getQueryString(void);
	public:
		CGI(void);
		CGI(std::string cgi_path, std::string path, HeadersBlock request, Configuration::server conf);
		~CGI(void);
		std::string getOutput(void);

};

#endif