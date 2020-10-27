#include "../includes/CGI.hpp"

/**
 * Creates a new CGI object
 * @param cgi_path the path to the CGI binary to execute
 * @param file the file to execute with the CGI
 * @param request the HTTP request to handle
 */
CGI::CGI(std::string cgi_path, std::string file, HeadersBlock request, Configuration::server conf)
: _cgi_path(cgi_path), _file(file), _request(request), _conf(conf)
{}

CGI::~CGI(void)
{}

/**
 * Executes and return the CGI
 * @return the string representation of the CGI
 */
std::string CGI::getOutput(void)
{
	std::map<std::string, std::string> args;

	args = _getParams();
	return ("");
}

/**
 * Get the CGI params
 * @return a map of the CGI param to execute
 */
std::map<std::string, std::string> CGI::_getParams(void)
{
	std::map<std::string, std::string> args;

	args["GATEWAY_INTERFACE"] = "CGI/1.1";
	args["PATH_INFO"] = _request.getRequestLine()._request_target;
	args["PATH_TRANSLATED"] = _file;
	args["QUERY_STRING"] = _getQueryString();
	std::cout << "query: " << args["QUERY_STRING"] << std::endl;
	args["REQUEST_METHOD"] = _request.getRequestLine()._method;
	args["SERVER_NAME"] = _conf.host;
	args["SERVER_PORT"] = _conf.port;
	args["SERVER_PROTOCOL"] = "HTTP/1.1";
	args["SERVER_SOFTWARE"] = "webserv";
	return (args);
}

/**
 * Get query string (everything after ? in url)
 * @return the query string
 * @example "index.php?a=42&page_id=32&c=21" -> "a=42&page_id=32&c=21"
 */
std::string CGI::_getQueryString(void)
{
	size_t i;

	i = 0;
	std::cout << "d " << _request.getRequestLine()._method << std::endl;
	while (_request.getRequestLine()._request_target[i] && _request.getRequestLine()._request_target[i] != '?')
		++i;
	if (_request.getRequestLine()._request_target[i] == '?')
		++i;
	return (std::string(_request.getRequestLine()._request_target, i, -i));
}