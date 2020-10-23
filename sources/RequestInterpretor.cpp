#include "../includes/RequestInterpretor.hpp"

/**
 * Creates a request interpretor from the request string and a server configuration
 * @param request the string repreesentation of the HTTP request
 * @param serverConf the server configuration
 * @todo Add the HTTP request parsing here
 */
RequestInterpretor::RequestInterpretor(std::string request, Configuration::server serverConf)
: _request(request), _conf(serverConf)
{
	this->_ressource = splitWhitespace(getLine(request, 0))[1];
	this->_location = _getLocation(_ressource);
	this->_ressource.replace(0, this->_location.name.size(), "/");
	DEBUG("location: " + this->_location.name);
}

RequestInterpretor::RequestInterpretor(const RequestInterpretor &other)
{
	*this = other;
}

RequestInterpretor::~RequestInterpretor(void)
{}

RequestInterpretor &RequestInterpretor::operator=(const RequestInterpretor &other)
{
	this->_ressource = other._ressource;
	this->_request = other._request;
	this->_conf = other._conf;
	this->_location = other._location;
	return (*this);
}

/**
 * Get the HTTP request for the given request
 * @return a string representing the HTTP response
 * @todo implement the case when the method isn't correct
 */
std::string RequestInterpretor::getResponse(void)
{
	std::string method = "GET";
	std::string ressource_path;

	ressource_path = _location.root;
	if (ressource_path[ressource_path.size() - 1] == '/')
		ressource_path = std::string(ressource_path, 0, ressource_path.size() - 1);
	ressource_path += _ressource;
	DEBUG("ressource path: " + ressource_path);
	if (method == "GET")
		return _get(ressource_path);
	return ("");
}

/**
 * Performs a GET request
 * @param ressource_path the path of the ressource to GET on the disk
 * @return the string representation of the HTTP response
 */
std::string RequestInterpretor::_get(std::string ressource_path)
{
	std::map<std::string, std::string> headers;
	std::vector<unsigned char> content_bytes;
	unsigned char *ressource_content;
	int ressource_type;

	headers["Content-Type"] = _getMIMEType(".html");
	ressource_type = pathType(ressource_path);
	if (ressource_type == 0)
		return (_generateResponse(404, headers, _getErrorHTMLPage(404)));
	if (ressource_type == 2)
	{
		if (pathType(ressource_path + _location.index) == 1)
		{
			ressource_path = ressource_path + _location.index;
			ressource_type = 1;
		}
		else
		{
			if (_location.autoindex)
				return (_generateResponse(200, headers, _getListingHTMLPage(ressource_path, _ressource)));
			else
				return (_generateResponse(403, headers, _getErrorHTMLPage(403)));
		}
	}
	if (ressource_type == 1)
	{
		try
		{
			content_bytes = readBinaryFile(ressource_path);
			ressource_content = reinterpret_cast<unsigned char *>(&content_bytes[0]);
			headers["Content-Type"] = _getMIMEType(ressource_path);
			return (_generateResponse(200, headers, ressource_content, content_bytes.size()));
		}
		catch (const std::exception &e)
		{
			return (_generateResponse(403, headers, _getErrorHTMLPage(403)));
		}
	}
	return (_generateResponse(500, headers, _getErrorHTMLPage(500)));
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
	response += "HTTP/1.1 ";
	response += uIntegerToString(code) + " ";
	response += _getStatusDescription(code) + "\n";
	it = headers.begin();
	while (it != headers.end())
	{
		response += it->first + ": " + it->second + "\n";
		++it;
	}
	response += "\n";
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
	struct dirent *en;
	DIR *dr;

	(void)path;
	base = readFile("./assets/listing.html");
	base = replace(base, "$1", ressource);
	dr = opendir(path.c_str());
	while ((en = readdir(dr)) != nullptr)
		listing += "<li><a href=\"" + ressource + "/" + std::string(en->d_name) +  "\">" + std::string(en->d_name) + "/</a></li>";
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

	i = 0;
	while (filename[i] && filename[i] != '.')
		++i;
	if (i >= filename.size())
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