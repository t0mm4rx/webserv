#include "../includes/CGI.hpp"

extern char **g_envp;

/**
 * Creates a new CGI object
 * @param cgi_path the path to the CGI binary to execute
 * @param ressource the file to execute with the CGI
 * @param request the HTTP request to handle
 * @param conf the server configuration
 * @param location the current location conf
 */
CGI::CGI(std::string cgi_path, std::string ressource_path, HeadersBlock request, Configuration::server conf, Configuration::location location)
: _cgi_path(cgi_path), _ressource_path(ressource_path), _request(request), _conf(conf), _location(location)
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
	try
	{ res = _execCGI(args_converted); }
	catch (std::exception & e)
	{ throwError(e); }
	return (res);
}

/**
 * Executes CGI, get its output
 * @param args the char** representation of CGI params
 * @throw CGIException if an error occures
 * @return the string output of the CGI
 */
std::string CGI::_execCGI(char **args)
{
	pid_t pid;
	int exec_res;
	char **exec_args;
	int tmp_fd;
	int fd[2];

	Log("Call CGI.");
	exec_args = _getExecArgs();
	if (pipe(fd) == -1)
		throw CGIException("Cannot create pip to execute CGI.");
	pid = fork();
	if (pid == 0)
	{
		close(fd[1]);
		dup2(fd[0], 0);
		tmp_fd = open("/tmp/webserv_cgi", O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
		if (tmp_fd < 0)
			throw CGIException("Cannot create temporary file to catch CGI output in /tmp.");
		dup2(tmp_fd, 1);
		dup2(tmp_fd, 2);
		exec_res = execve(_cgi_path.c_str(), exec_args, args);
		close(0);
		close(tmp_fd);
		close(fd[0]);
		exit(0);
	}
	else
	{
		close(fd[0]);
		write(fd[1], _request.getContent().c_str(), _request.getContent().length());
		close(fd[1]);
		waitpid(-1, NULL, 0);
		_freeArgs(args);
		_freeArgs(exec_args);
	}
	Log("End CGI");
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
 * @todo implement AUTH_TYPE, REMOTE_ADDR, REMOTE_IDENT, REMOTE_USER
 */
std::map<std::string, std::string> CGI::_getParams(void)
{
	std::map<std::string, std::string> args;
	std::string tmp;
	size_t i;
	size_t j;
	
	args["GATEWAY_INTERFACE"] = "CGI/1.1";
	args["PATH_TRANSLATED"] = _ressource_path;
	args["QUERY_STRING"] = _getQueryString();
	args["REQUEST_METHOD"] = _request.getRequestLine()._method;
	args["CONTENT_LENGTH"] = uIntegerToString(_request.getContent().length());
	for (size_t u = 0; u < _request.getHeaderFields().size(); u++)
		if (_request.getHeaderFields()[u]._field_name == "Content-Type")
		{
			DEBUG("CONTENT GIVEN = " << _request.getHeaderFields()[u]._field_value)
			args["CONTENT_TYPE"] = _request.getHeaderFields()[u]._field_value;
		}
	args["REQUEST_URI"] = _removeQueryArgs(_request.getRequestLine()._request_target);
	args["REMOTE_IDENT"] = "";
	args["REDIRECT_STATUS"] = "200";
	args["REMOTE_ADDR"] = _request.getClientIP();
	args["SCRIPT_NAME"] = _location.name + ((_location.name[_location.name.length() - 1] == '/') ? "" : "/") + replace(_ressource_path, _location.root, "");
	args["PATH_INFO"] = _removeQueryArgs(_request.getRequestLine()._request_target);
	args["SCRIPT_FILENAME"] = _ressource_path;
	args["SERVER_NAME"] = _conf.host;
	args["SERVER_PORT"] = uIntegerToString(_conf.port);
	args["SERVER_PROTOCOL"] = "HTTP/1.1";
	args["SERVER_SOFTWARE"] = "webserv/1.0";
	for (size_t a = 0; a < _request.getHeaderFields().size(); ++a)
	{
		tmp = _request.getHeaderFields()[a]._field_name;
		tmp = replace(tmp, "-", "_");
		for (size_t b = 0; b < tmp.size(); ++b)
			tmp[b] = toupper(tmp[b]);
		args["HTTP_" + tmp] = _request.getHeaderFields()[a]._field_value;
	}
	i = 0;
	while (g_envp[i])
	{
		j = 0;
		while (g_envp[i][j] && g_envp[i][j] != '=')
			j++;
		args[std::string(g_envp[i], 0, j)] = std::string(g_envp[i], j + 1, std::string(g_envp[i]).size() - j);
		++i;
	}
	return (args);
}

std::string CGI::_getScriptName(void)
{
	size_t i;

	i = 0;
	while (_request.getRequestLine()._request_target[i] && _request.getRequestLine()._request_target[i] != '?')
		++i;
	return (std::string(_request.getRequestLine()._request_target, 0, i));
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

/**
 * Remove query
 * @param query the string to remove query from
 * @return the string without query params
 * @example "/php/index.php?a=1&page_id=2" -> "/php/index.php"
 */
std::string CGI::_removeQueryArgs(std::string query)
{
	size_t i;

	i = 0;
	while (query[i] && query[i] != '?')
		++i;
	return (std::string(query, 0, i));
}