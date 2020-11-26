#include "../includes/RequestInterpretor.hpp"

/**
 * Creates a request interpretor from the request string and a server configuration
 * @param request the string repreesentation of the HTTP request
 * @param serverConf the server configuration
 */
RequestInterpretor::RequestInterpretor(HeadersBlock & header_block, Configuration::server serverConf)
: _header_block(header_block), _conf(serverConf)
{
	if (this->_header_block.isRequest())
		this->_ressource = this->_header_block.getRequestLine()._request_target;
	this->_location = _getLocation(_ressource);
	this->_ressource = _formatRessource(this->_ressource);
	DEBUG("location: " + this->_location.name);
}

RequestInterpretor::RequestInterpretor(const RequestInterpretor &other)
: _header_block(other._header_block)
{
	*this = other;
}

RequestInterpretor::~RequestInterpretor(void)
{}

RequestInterpretor &RequestInterpretor::operator=(const RequestInterpretor &other)
{
	this->_header_block = other._header_block;
	this->_ressource = other._ressource;
	this->_conf = other._conf;
	this->_location = other._location;
	return (*this);
}

/**
 * Translate path and get the HTTP request for the given request
 * @return a string representing the HTTP response
 * @todo implement the case when the method isn't correct
 */
std::string RequestInterpretor::getResponse(void)
{
	std::map<std::string, std::string> headers;
	std::string method = _header_block.getRequestLine()._method;
	std::string ressource_path;

	headers["Content-Type"] = _getMIMEType("a.html");
	if (_header_block.getContent().size() > _location.client_max_body_size)
		return (_generateResponse(413, headers, method != "HEAD" ? _getErrorHTMLPage(413) : ""));
	if (!_isMethodAllowed(method))
		return (_wrongMethod());
	if (method == "TRACE")
		return (_trace(headers));
	else if (method == "OPTIONS")
		return (_options(headers));
	else if (method == "CONNECT")
		return (_generateResponse(200, headers, ""));
	ressource_path = _location.root;
	if (ressource_path[ressource_path.size() - 1] == '/')
		ressource_path = std::string(ressource_path, 0, ressource_path.size() - 1);
	ressource_path += _ressource;
	DEBUG("ressource path: " + ressource_path);
	if (pathType(ressource_path, NULL) == 2)
	{
		DEBUG("ressource path: " + ressource_path + ((ressource_path[ressource_path.length() - 1] == '/') ? "" : "/") + _location.index);
		if (_location.index.length() > 0)
			ressource_path = ressource_path + ((ressource_path[ressource_path.length() - 1] == '/') ? "" : "/") + _location.index;
		else
		{
			if (_location.autoindex)
				return (_generateResponse(200, headers, method != "HEAD" ? _getListingHTMLPage(ressource_path, _ressource) : ""));
			else
				return (_generateResponse(403, headers, method != "HEAD" ? _getErrorHTMLPage(403) : ""));
		}
	}
	if (pathType(ressource_path, NULL) == 0 && method != "PUT" && method != "POST")
		return (_generateResponse(404, headers, method != "HEAD" ? _getErrorHTMLPage(404) : ""));
	if (_shouldCallCGI(ressource_path))
	{
		DEBUG("call CGI for this request");
		try
		{
			return (_addCGIHeaders(CGI(_location.cgi_path, ressource_path, _header_block, _conf, _location).getOutput()));
		}
		catch (const std::exception &e)
		{
			std::cerr << e.what() << std::endl;
			return (_generateResponse(500, headers, method != "HEAD" ?_getErrorHTMLPage(500) : ""));
		}
	}
	if (method == "GET")
		return _get(ressource_path, headers);
	else if (method == "HEAD")
		return _head(ressource_path, headers);
	else if (method == "POST")
		return _post(ressource_path, headers);
	else if (method == "PUT")
		return (_put(ressource_path, headers));
	else if (method == "DELETE")
		return (_delete(ressource_path, headers));
	return ("");
}

/**
 * Performs a GET request
 * @param ressource_path the path of the ressource to GET on the disk
 * @return the string representation of the HTTP response
 */
std::string RequestInterpretor::_get(std::string ressource_path, std::map<std::string, std::string> headers, bool send_body)
{
	std::vector<unsigned char> content_bytes;
	unsigned char *ressource_content;
	time_t file_date;
	try
	{
		content_bytes = readBinaryFile(ressource_path);
		ressource_content = reinterpret_cast<unsigned char *>(&content_bytes[0]);
		headers["Content-Type"] = _getMIMEType(ressource_path);
		pathType(ressource_path, &file_date);
		headers["Last-Modified"] = _formatTimestamp(file_date);
		if (send_body)
			return (_generateResponse(200, headers, ressource_content, content_bytes.size()));
		return (_generateResponse(200, headers, ""));
	}
	catch (const std::exception &e)
	{
		return (_generateResponse(403, headers, send_body ? _getErrorHTMLPage(403) : ""));
	}
	return (_generateResponse(500, headers, send_body ? _getErrorHTMLPage(500) : ""));
}

/**
 * Performs a HEAD request
 * @param ressource_path the path of the ressource to GET on the disk
 * @return the string representation of the HTTP response
 */
std::string RequestInterpretor::_head(std::string ressource_path, std::map<std::string, std::string> headers)
{
	return (_get(ressource_path, headers, false));
}

/**
 * 	@brief Performs a PUT request.
 *	@param ressource_path the path of the ressource to GET on the disk
 * 	@return the string representation of the HTTP response
 */
std::string RequestInterpretor::_post(std::string ressource_path, std::map<std::string, std::string> headers)
{
	// struct stat   buffer;
	int fd = -1;
	int rtn = 0;
	int type;
	std::string path;

	if (_location.upload_path.size() > 0)
	{
		std::string file = std::string(_header_block.getRequestLine()._request_target, _location.name.size());
		path = _location.upload_path + "/" + file;
	}
	else
		path = ressource_path;
	DEBUG("POST path: " + path);
	type = pathType(path, NULL);
	try
	{
		if (type == 1)
		{
			if ((fd = open(path.c_str(), O_WRONLY | O_TRUNC, 0644)) == -1)
				throw(throwMessageErrno("TO CHANGE"));
			write(fd, _header_block.getContent().c_str(), _header_block.getContent().length());
			close(fd);
			rtn = 200;
			headers["Content-Location"] = _header_block.getRequestLine()._request_target;
		}
		else if (type == 0)
		{
			if ((fd = open(path.c_str(), O_WRONLY | O_APPEND | O_CREAT, 0644)) == -1)
				return (_generateResponse(500, headers, _getErrorHTMLPage(500)));
			write(fd, _header_block.getContent().c_str(), _header_block.getContent().length());
			close(fd);
			rtn = 201;
			headers["Location"] = _header_block.getRequestLine()._request_target;
		}
		else
			return (_generateResponse(500, headers, _getErrorHTMLPage(500)));
	}
	catch (std::exception & ex)
	{
		throwError(ex);
	}
	return (_generateResponse(rtn, headers, ""));
}

/**
 * 	@brief Performs a PUT request.
 *	@param ressource_path the path of the ressource to GET on the disk
 * 	@return the string representation of the HTTP response
 */
std::string RequestInterpretor::_put(std::string ressource_path, std::map<std::string, std::string> headers)
{
	int fd = -1;
	int rtn = 0;
	int type;
	std::string path;

	if (_location.upload_path.size() > 0)
	{
		std::string file = std::string(_header_block.getRequestLine()._request_target, _location.name.size());
		path = _location.upload_path + "/" + file;
	}
	else
		path = ressource_path;
	DEBUG("PUT path: " + path);
	type = pathType(path, NULL);
	try
	{
		if (type == 0)
		{
			if ((fd = open(path.c_str(), O_CREAT | O_WRONLY | O_TRUNC, 0644)) == -1)
				throw(throwMessageErrno("TO CHANGE"));
			write(fd, _header_block.getContent().c_str(), _header_block.getContent().length());
			close(fd);
			rtn = 201;
		}
		else if (type == 1)
		{
			if ((fd = open(path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644)) == -1)
				throw(throwMessageErrno("Create file on put"));
			write(fd, _header_block.getContent().c_str(), _header_block.getContent().length());
			close(fd);
			rtn = 204;
		}
		else
			return (_generateResponse(500, headers, _getErrorHTMLPage(500)));
		headers["Content-Location"] = _header_block.getRequestLine()._request_target;
	}
	catch (std::exception & ex)
	{
		throwError(ex);
		return (_generateResponse(500, headers, _getErrorHTMLPage(500)));
	}
	return (_generateResponse(rtn, headers, ""));
}

/**
 * 	Performs a DELETE request.
 *	@param ressource_path the path of the ressource to GET on the disk
 * 	@return the string representation of the HTTP response
 */
std::string RequestInterpretor::_delete(std::string ressource_path, std::map<std::string, std::string> headers)
{
	int type;

	type = pathType(ressource_path, NULL);
	if (type == 1)
	{
		unlink(ressource_path.c_str());
		return (_generateResponse(200, headers, ""));
	}
	return (_generateResponse(404, headers, _getErrorHTMLPage(404)));
}

/**
 * Performs a TRACE request.
 * @param headers a map representing HTTP headers of the reponse.
 * @return the string representation of the HTTP response
 */
std::string RequestInterpretor::_trace(std::map<std::string, std::string> headers)
{
	headers["Content-Type"] = "message/http";
	return (_generateResponse(200, headers, _header_block.getPlainRequest()));
}

/**
 * Performs an OPTIONS request.
 * @param headers a map representing HTTP headers of the reponse.
 * @return the string representation of the HTTP response
 */
std::string RequestInterpretor::_options(std::map<std::string, std::string> headers)
{
	headers.erase("Content-Type");
	std::string allowed;

	for (size_t i = 0; i < _location.methods.size(); ++i)
	{
		allowed += _location.methods[i];
		if (i < _location.methods.size() - 1)
			allowed += ", ";
	}
	headers["Allow"] = allowed;
	return (_generateResponse(200, headers, ""));
}

/**
 * Returns a 405 response in case of not allowed method
 * @return the string representation of the HTTP response
 */
std::string RequestInterpretor::_wrongMethod(void)
{
	std::map<std::string, std::string> headers;
	std::string allowed;

	for (size_t i = 0; i < _location.methods.size(); ++i)
	{
		allowed += _location.methods[i];
		if (i < _location.methods.size() - 1)
			allowed += ", ";
	}
	headers["Allow"] = allowed;
	return (_generateResponse(405, headers, _header_block.getRequestLine()._method != "HEAD" ?_getErrorHTMLPage(405) : ""));
}

/**
 * Creates a HTTP response based on given code and content
 * @param code the status code of the response
 * @param headers headers to inject in response
 * @param content an unsigned char array of the content to send
 * @param size the size in bytes of the content
 * @return the string representation of a HTTP response
 */
std::string RequestInterpretor::_generateResponse(size_t code, std::map<std::string, std::string> headers, const unsigned char *content, size_t content_size)
{
	std::string response;
	std::map<std::string, std::string>::iterator it;

	headers["Content-Length"] = uIntegerToString(content_size);
	headers["Server"] = "webserv";
	headers["Date"] = _getDateHeader();
	response += "HTTP/1.1 ";
	response += uIntegerToString(code) + " ";
	response += _getStatusDescription(code) + "\r\n";
	it = headers.begin();
	while (it != headers.end())
	{
		response += it->first + ": " + it->second + "\r\n";
		++it;
	}
	response += "\r\n";
	for (size_t i = 0; i < content_size; ++i)
		response += content[i];
	return (response);
}

/**
 * Creates a HTTP response based on given code and ASCII content
 * @param code the status code of the response
 * @param headers headers to inject in response
 * @param content the string representation of the content
 * @return the string representation of a HTTP response
 */
std::string RequestInterpretor::_generateResponse(size_t code, std::map<std::string, std::string> headers, std::string content)
{
	return (_generateResponse(code, headers, reinterpret_cast<const unsigned char *>(content.c_str()), content.size()));
}

/**
 * Get the status description following the rfc 7231 section 6.1
 * @param code the HTTP status code
 * @return the corresponding reason description
 */
std::string RequestInterpretor::_getStatusDescription(size_t code)
{
	std::map<std::size_t, std::string> m;

	m[100] = "Continue";
	m[101] = "Switching Protocols";
	m[200] = "OK";
	m[201] = "Created";
	m[202] = "Accepted";
	m[203] = "Non-Authoritative Information";
	m[204] = "No Content";
	m[205] = "Reset Content";
	m[206] = "Partial Content";
	m[300] = "Multiple Choices";
	m[301] = "Moved Permanently";
	m[302] = "Found";
	m[303] = "See Other";
	m[304] = "Not Modified";
	m[305] = "Use Proxy";
	m[307] = "Temporary Redirect";
	m[400] = "Bad Request";
	m[401] = "Unauthorized";
	m[402] = "Payment Required";
	m[403] = "Forbidden";
	m[404] = "Not Found";
	m[405] = "Method Not Allowed";
	m[406] = "Not Acceptable";
	m[407] = "Proxy Authentication Required";
	m[408] = "Request Timeout";
	m[409] = "Conflict";
	m[410] = "Gone";
	m[411] = "Length Required";
	m[412] = "Precondition Failed";
	m[413] = "Payload Too Large";
	m[414] = "URI Too Long";
	m[415] = "Unsupported Media Type";
	m[416] = "Range Not Satisfiable";
	m[417] = "Expectation Failed";
	m[426] = "Upgrade Required";
	m[500] = "Internal Server Error";
	m[501] = "Not Implemented";
	m[502] = "Bad Gateway";
	m[503] = "Service Unavailable";
	m[504] = "Gateway Timeout";
	m[505] = "HTTP Version Not Supported";
	return m[code];
	return ("Error");
}

/**
 * Get the HTML page for a given status error
 * @param status the status of the response
 * @return a HTML page describing the error
 */
std::string RequestInterpretor::_getErrorHTMLPage(size_t code)
{
	std::string base;

	if (_conf.error_pages.count(code) > 0)
		return (readFile(_conf.error_pages[code]));
	base = readFile("./assets/error.html");
	base = replace(base, "$1", uIntegerToString(code));
	base = replace(base, "$2", _getStatusDescription(code));
	return (base);
}

/**
 * Get the directory listing HTML page
 * @param path the full path of the directory to list on the disk
 * @param ressource the ressource the user tried to reach
 * @return a HTML page that lists the content of the given directory
 */
std::string RequestInterpretor::_getListingHTMLPage(std::string path, std::string ressource)
{
	std::string base;
	std::string listing;
	std::string link_base;
	size_t i;
	struct dirent *en;
	DIR *dr;

	base = readFile("./assets/listing.html");
	base = replace(base, "$1", ressource);
	dr = opendir(path.c_str());
	i = 0;
	while (_header_block.getRequestLine()._request_target[i] && _header_block.getRequestLine()._request_target[i] != '?')
		link_base += _header_block.getRequestLine()._request_target[i++];
	if (link_base[link_base.size() - 1] != '/')
		link_base += '/';
	while ((en = readdir(dr)) != 0)
		listing += "<li><a href=\"" + link_base + std::string(en->d_name) +  "\">" + std::string(en->d_name) + "</a></li>";
	closedir(dr);
	base = replace(base, "$2", listing);
	return (base);
}

/**
 * Get the content type based on filename
 * @param filename
 * @return the MIME type for the given filename
 */
std::string RequestInterpretor::_getMIMEType(std::string filename)
{
	std::map<std::string, std::string> m;
	std::string ext;
	size_t i;

	i = filename.size() - 1;
	while (i > 0 && filename[i] != '.')
		--i;
	if (i == 0)
		return ("text/plain");
	ext = std::string(filename, i + 1, filename.size() - i);
	m["aac"] = "audio/aac";
	m["abw"] = "application/x-abiword";
	m["arc"] = "application/octet-stream";
	m["avi"] = "video/x-msvideo";
	m["azw"] = "application/vnd.amazon.ebook";
	m["bin"] = "application/octet-stream";
	m["bmp"] = "image/bmp";
	m["bz"] = "application/x-bzip";
	m["bz2"] = "application/x-bzip2";
	m["csh"] = "application/x-csh";
	m["css"] = "text/css";
	m["csv"] = "text/csv";
	m["doc"] = "application/msword";
	m["docx"] = "application/vnd.openxmlformats-officedocument.wordprocessingml.document";
	m["eot"] = "application/vnd.ms-fontobject";
	m["epub"] = "application/epub+zip";
	m["gif"] = "image/gif";
	m["htm"] = "text/html";
	m["html"] = "text/html";
	m["ico"] = "image/x-icon";
	m["ics"] = "text/calendar";
	m["jar"] = "application/java-archive";
	m["jpeg"] = "image/jpeg";
	m["jpg"] = "image/jpeg";
	m["js"] = "application/javascript";
	m["json"] = "application/json";
	m["mid"] = "audio/midi";
	m["midi"] = "audio/midi";
	m["mpeg"] = "video/mpeg";
	m["mpkg"] = "application/vnd.apple.installer+xml";
	m["odp"] = "application/vnd.oasis.opendocument.presentation";
	m["ods"] = "application/vnd.oasis.opendocument.spreadsheet";
	m["odt"] = "application/vnd.oasis.opendocument.text";
	m["oga"] = "audio/ogg";
	m["ogv"] = "video/ogg";
	m["ogx"] = "application/ogg";
	m["otf"] = "font/otf";
	m["png"] = "image/png";
	m["pdf"] = "application/pdf";
	m["ppt"] = "application/vnd.ms-powerpoint";
	m["pptx"] = "application/vnd.openxmlformats-officedocument.presentationml.presentation";
	m["rar"] = "application/x-rar-compressed";
	m["rtf"] = "application/rtf";
	m["sh"] = "application/x-sh";
	m["svg"] = "image/svg+xml";
	m["swf"] = "application/x-shockwave-flash";
	m["tar"] = "application/x-tar";
	m["tif"] = "image/tiff";
	m["tiff"] = "image/tiff";
	m["ts"] = "application/typescript";
	m["ttf"] = "font/ttf";
	m["vsd"] = "application/vnd.visio";
	m["wav"] = "audio/x-wav";
	m["weba"] = "audio/webm";
	m["webm"] = "video/webm";
	m["webp"] = "image/webp";
	m["woff"] = "font/woff";
	m["woff2"] = "font/woff2";
	m["xhtml"] = "application/xhtml+xml";
	m["xls"] = "application/vnd.ms-excel";
	m["xlsx"] = "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet";
	m["xml"] = "application/xml";
	m["xul"] = "application/vnd.mozilla.xul+xml";
	m["zip"] = "application/zip";
	m["3gp"] = "audio/3gpp";
	m["3g2"] = "audio/3gpp2";
	m["7z"] = "application/x-7z-compressed";
	if (m.count(ext))
		return (m[ext]);
	return ("application/octet-stream");
}

/**
 * Get the right location in configuration based on the asked ressource
 * @param ressource the asked ressource
 * @return the location configuration object
 * @example ressource "/" in configurations "/wordpress", "/upload" and "/" will return "/"
 * @example ressource "/wordpress/index.php" in configurations "/wordpress", "/upload" and "/" will return "/wordpress"
 */
Configuration::location RequestInterpretor::_getLocation(std::string ressource)
{
	size_t max_length;
	size_t max_index;

	for (size_t i = 0; i < _conf.locations.size(); ++i)
	{
		if (_conf.locations[i].name == ressource)
			return (_conf.locations[i]);
	}
	max_length = 0;
	max_index = 0;
	for (size_t i = 0; i < _conf.locations.size(); ++i)
	{
		if (ressource.rfind(_conf.locations[i].name, 0) == 0)
		{
			if (_conf.locations[i].name.size() > max_length)
			{
				max_length = _conf.locations[i].name.size();
				max_index = i;
			}
		}
	}
	return (_conf.locations[max_index]);
}

/**
 * Get the current HTTP formatted date
 * @return a string representing the current date formatted for HTTP header
 */
std::string RequestInterpretor::_getDateHeader(void)
{
	struct timeval now;
	struct timezone tz;

	gettimeofday(&now, &tz);
	return (_formatTimestamp(now.tv_sec + tz.tz_minuteswest * 60));
}

/**
 * Format a timestamp to a HTTP formatted date
 * @param timestamp the timestamp in second of the date
 * @return a string representation of the date according to HTTP standard
 */
std::string RequestInterpretor::_formatTimestamp(time_t timestamp)
{
	char buffer[33];
	struct tm *ts;
	size_t last;

	ts   = localtime(&timestamp);
	last = strftime(buffer, 32, "%a, %d %b %Y %T GMT", ts);
	buffer[last] = '\0';
	return (std::string(buffer));
}

/**
 * Is a method available on current location
 * @param method the HTTP method
 * @return wether the method is accepted on this location or not
 */
bool RequestInterpretor::_isMethodAllowed(std::string method)
{
	for (size_t i = 0; i < _location.methods.size(); ++i)
	{
		if (_location.methods[i] == method)
			return (true);
	}
	return (false);
}

/**
 * Remove location name and arguments from ressource
 * @param ressource
 * @return ressource without location name and args
 * @example "/wordpress/index.php?page_id=12" with location "/wordpress" will give "/index.php"
 */
std::string RequestInterpretor::_formatRessource(std::string ressource)
{
	std::string res;
	size_t i;

	i = 0;
	res = ressource;
	res.replace(0, this->_location.name.size(), "/");
	res = replace(res, "//", "/");
	while (res[i] && res[i] != '?')
		++i;
	res = std::string(res, 0, i);
	return (res);
}

/**
 * Detect if we should use a CGI for this file
 * @return wether we should use the CGI for the current request
 */
bool RequestInterpretor::_shouldCallCGI(std::string ressource_path)
{
	size_t i;
	std::string ext;


	if (_location.cgi_path.size() == 0)
		return (false);
	i = ressource_path.size() - 1;
	while (i > 0 && ressource_path[i] != '.')
		--i;
	if (i >= ressource_path.size())
		return (false);
	ext = std::string(ressource_path, i + 1, ressource_path.size() - i);
	for (size_t j = 0; j < _location.cgi_extension.size(); ++j)
	{
		if (_location.cgi_extension[j] == ext)
			return (true);
	}
	return (false);
}

/**
 * Add mendatory server headers to a CGI HTTP response, if status is returned we get it to form HTTP status
 * @param response the HTTP response out of the CGI
 * @return the same HTTP response with additional headers
 */
std::string RequestInterpretor::_addCGIHeaders(std::string response)
{
	std::string res;
	size_t size;
	int header_char_count = 0;

	std::string headers = response;
	if (response.find("\r\n\r\n") != std::string::npos)
		headers = response.substr(0, response.find("\r\n\r\n"));
	if (headers != "")
	{
		for (size_t i = 0; i < headers.length(); i++)
			if (headers[i] != '\n' && headers[i] != '\r')
				header_char_count++;
	}
	size = response.size() - std::count(response.begin(), response.end(), '\n') - std::count(response.begin(), response.end(), '\r') - header_char_count;
	res = response;
	res = "Content-Length: " + uIntegerToString(size) + "\r\n" + res;
	res = "Date: " + _getDateHeader() + "\r\n" + res;
	if (_getCGIStatus(response).size() > 0)
		res = "HTTP/1.1 " + _getCGIStatus(response) + "\r\n" + res;
	else
		res = "HTTP/1.1 200 OK\r\n" + res;
	return (res);
}

/**
 * Get CGI response status if any
 * @param response the CGI response
 * @return a string containing the code and status if found, an empty string if not
 */
std::string RequestInterpretor::_getCGIStatus(std::string response)
{
	std::vector<std::string> splits;

	for (size_t i = 0; i < countLines(response); ++i)
	{
		splits = splitWhitespace(getLine(response, i));
		if (splits.size() == 3 && splits[0] == "Status:")
			return (splits[1] + " " + splits[2]);
	}
	return ("");
}