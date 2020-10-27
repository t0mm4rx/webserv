#include "../includes/CGI.hpp"

/**
 * Creates a new CGI object
 * @param cgi_path the path to the CGI binary to execute
 * @param file the file to execute with the CGI
 * @param request the HTTP request to handle
 */
CGI::CGI(std::string cgi_path, std::string ressource_path, HeadersBlock request, Configuration::server conf)
: _cgi_path(cgi_path), _ressource_path(ressource_path), _request(request), _conf(conf)
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
	char **args_converted;
	std::string res;

	args = _getParams();
	args_converted = _convertParams(args);
	res = _execCGI(args_converted);
	DEBUG("cgi output:\n" + res);
	return (res);
}

/**
 * Executes CGI, get its output
 * @param args the char** representation of CGI params
 * @return the string output of the CGI
 */
std::string CGI::_execCGI(char **args)
{
	pid_t pid;
	int exec_res;
	char **exec_args;
	int tmp_fd;

	exec_args = _getExecArgs();
	pid = fork();
	if (pid == 0)
	{
		tmp_fd = open("/tmp/webserv_cgi", O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
		if (tmp_fd < 0)
			return ("Error");
		// dup2(tmp_fd, 1);
		// dup2(tmp_fd, 2);
		DEBUG("Here1 ");
		exec_res = execve(_cgi_path.c_str(), exec_args, args);
		DEBUG("Here");
		close(tmp_fd);
		exit(0);
	}
	else
	{
		waitpid(-1, NULL, 0);
		_freeArgs(args);
		_freeArgs(exec_args);
	}
	return (readFile("/tmp/webserv_cgi"));
}

/**
 * Get exec args (script name and file to treat)
 * @return char ** of execve args
 */
char **CGI::_getExecArgs(void)
{
	char **args;

	if (!(args = (char **)malloc(sizeof(char *) * 3)))
		return (0);
	args[0] = _newStr(_cgi_path);
	args[1] = _newStr(_ressource_path);
	args[2] = 0;
	return (args);
}

/**
 * Free the CGI args array
 * @param args the char ** to free
 */
void CGI::_freeArgs(char **args)
{
	size_t i;

	i = 0;
	while (args[i])
		free(args[i++]);
	free(args);
}

/**
 * Get the char** representation of CGI arguments
 * @param args arguments to convert
 * @return a char** that represents CGI aguments, should be free later
 */
char **CGI::_convertParams(std::map<std::string, std::string> args)
{
	char **result;
	std::map<std::string, std::string>::iterator it;
	size_t i;

	if (!(result = (char **)malloc(sizeof(char *) * (args.size() + 1))))
		return (0);
	it = args.begin();
	i = 0;
	while (it != args.end())
	{
		result[i++] = _newStr(it->first + "=" + it->second);
		++it;
	}
	result[args.size()] = 0;
	return (result);
}

/**
 * Get a char* from a std::string
 * @param source the string to copy
 * @return char* representing the source string, needs to be freed later
 */
char *CGI::_newStr(std::string source)
{
	char *res;

	if (!(res = (char *)malloc(sizeof(char) * (source.size() + 1))))
		return (0);
	for (size_t i = 0; i < source.size(); ++i)
		res[i] = source[i];
	res[source.size()] = 0;
	return (res);
}

/**
 * Get the CGI params
 * @return a map of the CGI param to execute
 */
std::map<std::string, std::string> CGI::_getParams(void)
{
	std::map<std::string, std::string> args;

	args["CONTENT_LENGTH"] = "0";
	args["GATEWAY_INTERFACE"] = "CGI/1.1";
	args["PATH_INFO"] = _ressource_path;
	args["PATH_TRANSLATED"] = _ressource_path;
	args["QUERY_STRING"] = _getQueryString();
	args["REQUEST_METHOD"] = _request.getRequestLine()._method;
	args["REQUEST_URI"] = _ressource_path;
	args["SCRIPT_NAME"] = _ressource_path;
	args["SERVER_NAME"] = _conf.host;
	args["SERVER_PORT"] = _conf.port;
	args["SERVER_PROTOCOL"] = "HTTP/1.1";
	args["SERVER_SOFTWARE"] = "webserv/1.0";
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
	while (_request.getRequestLine()._request_target[i] && _request.getRequestLine()._request_target[i] != '?')
		++i;
	if (_request.getRequestLine()._request_target[i] == '?')
		++i;
	return (std::string(_request.getRequestLine()._request_target, i, -i));
}