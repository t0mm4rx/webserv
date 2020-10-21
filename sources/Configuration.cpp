#include "../includes/Configuration.hpp"

Configuration::Configuration(void)
{}

Configuration::Configuration(std::string file)
{
	_parseConfig(file);
}

Configuration &Configuration::operator=(const Configuration &other)
{
	this->_servers = other._servers;
	return (*this);
}

Configuration::~Configuration(void)
{}

/**
* Parse a file into the current configuration object.
* @throw Configuration::ParsingException
* @param file path to the file to parse
*/
void Configuration::_parseConfig(std::string file)
{
	std::string file_content;
	size_t i;
	size_t size;
	std::vector<std::string> line;

	i = 0;
	file_content = readFile(file);
	size = countLines(file_content);
	while (i < size)
	{
		if (!isSkippable(file_content, i))
		{
			line = splitWhitespace(getLine(file_content, i));
			if (line.size() > 0 && line[0] == "server")
			{
				_parseServer(file_content, i, getClosingBracket(file_content, i));
				i = getClosingBracket(file_content, i);
			}
			else
				throw ParsingException(i, "Unexpected token '" + line[0] + "'.");
		}
		++i;
	}
}

/**
* Add a server structure based on lines given
* @param source the config string
* @param line_start the line where server definition starts, aka "server {"
* @param line_end the line where server definition ends, aka "}"
*/
void Configuration::_parseServer(std::string source, size_t line_start, size_t line_end)
{
	server s;
	std::vector<std::string> line;

	for (size_t n = line_start + 1; n < line_end; ++n)
	{
		if (!isSkippable(source, n))
		{
			std::vector<std::string> words = splitWhitespace(getLine(source, n));
			if (words.size() > 0 && words[0] == "location")
			{
				s.locations.push_back(_parseLocation(source, n, getClosingBracket(source, n)));
				n = getClosingBracket(source, n);
			}
			else
				_parseServerProperty(source, n, s);
		}
	}
	_servers.push_back(s);
}

/**
* Takes a line containing a property, and set the right property of s
* @param source the config string
* @param line the line of the property
* @param s reference to the server to edit property to
*/
void Configuration::_parseServerProperty(std::string source, size_t n, server &s)
{
	std::vector<std::string> line;
	char last;
	
	line = parseProperty(source, n, "server");
	if (line[0] == server_properties[0])
	{
		if (line.size() != 3)
			throw ParsingException(n, std::string(server_properties[0]) + " <port> <host>;");
		s.port = uIntegerParam(line[1], n);
		s.host = line[2];
	}
	if (line[0] == server_properties[1])
	{
		if (line.size() != 2)
			throw ParsingException(n, std::string(server_properties[1]) + " <name>;");
		s.name = line[1];
	}
	if (line[0] == server_properties[2])
	{
		if (line.size() != 3)
			throw ParsingException(n, std::string(server_properties[2]) + "<code> <file>;");
		s.error_pages[uIntegerParam(line[1], n)] = line[2];	
	}
	if (line[0] == server_properties[3])
	{
		if (line.size() != 2)
			throw ParsingException(n, std::string(server_properties[3]) + " <size[K,M,G]>;");
		s.client_max_body_size = uIntegerParam(line[1], n);
		last = line[1][line[1].size() - 1];
		if (last == 'K' || last == 'k')
			s.client_max_body_size *= 1024;
		else if (last == 'M' || last == 'm')
			s.client_max_body_size *= 1024 * 1024;
		else if (last == 'G' || last == 'G')
			s.client_max_body_size *= 1024 * 1024 * 1024;
		else if (!std::isdigit(last))
			throw ParsingException(n, std::string(server_properties[3]) + " <size[K,M,G]>;");
	}
}

/**
* Return a location based on the given string
* @param source the config string
* @param line_start the line where location definition starts, aka "location / {"
* @param line_end the line where location definition ends, aka "}"
* @return a location struct based on configuration given
*/
Configuration::location Configuration::_parseLocation(std::string source, size_t line_start, size_t line_end)
{
	location loc;
	std::vector<std::string> line;

	line = splitWhitespace(getLine(source, line_start));
	if (line.size() != 3)
		throw ParsingException(line_start, "Location should have a name.");
	loc.name = line[1];
	for (size_t n = line_start + 1; n < line_end; ++n)
	{
		if (!isSkippable(source, n))
			_parseLocationProperty(source, n, loc);
	}
	return (loc);
}

/**
* Takes a line with a location property, and set it to the given l location struct
* @param source the config string
* @param line the line to read the property from
* @param l a reference to the location struct to edit
*/
void Configuration::_parseLocationProperty(std::string source, size_t n, location &l)
{
	std::vector<std::string> line;

	line = parseProperty(source, n, "route");
	if (line[0] == route_properties[0])
	{
		for (size_t i = 1; i < line.size(); ++i)
		{
			if (!isMethodValid(line[i]))
				throw ParsingException(n, "'" + line[i] + "' is not a valid method.");
			else
				l.methods.push_back(line[i]);
		}
	}
	if (line[0] == route_properties[1])
		l.root = line[1];
	if (line[0] == route_properties[2])
		l.autoindex = boolParam(line[1], n);
	if (line[0] == route_properties[3])
		l.index = line[1];
	if (line[0] == route_properties[4])
		l.cgi_extension = line[1];
	if (line[0] == route_properties[5])
		l.cgi_path = line[1];
	if (line[0] == route_properties[6])
		l.upload_enable = boolParam(line[1], n);
	if (line[0] == route_properties[7])
		l.upload_path = line[1];
}

/**
* Prints an entire configuration to ensure parsing is good
*/
void Configuration::print(void)
{
	std::map<int, std::string>::iterator it;
	std::vector<location>::iterator it2;

	for (size_t i = 0; i < _servers.size(); i++)
	{
		std::cout << "- Server" << std::endl;
		std::cout << "   * server_name: " + _servers[i].name << std::endl;
		std::cout << "   * host: " + _servers[i].host << std::endl;
		// std::cout << "   * port: " + std::to_string(_servers[i].port) << std::endl;
		// std::cout << "   * client_max_body_size: " + std::to_string(_servers[i].client_max_body_size) << std::endl;
		it = _servers[i].error_pages.begin();
		while (it != _servers[i].error_pages.end())
		{
			// std::cout << "   * error_page for " + std::to_string(it->first) + ": " + it->second << std::endl;
			++it;
		}
		it2 = _servers[i].locations.begin();
		while (it2 != _servers[i].locations.end())
		{
			std::cout << "   - Location " + it2->name << std::endl;
			std::cout << "     * methods: ";
			for (size_t j = 0; j < it2->methods.size(); ++j)
				std::cout << it2->methods[j] + " ";
			std::cout << std::endl;
			std::cout << "     * root: " << it2->root << std::endl;
			std::cout << "     * cgi_extension: " << it2->cgi_extension << std::endl;
			std::cout << "     * cgi_path: " << it2->cgi_path << std::endl;
			std::cout << "     * autoindex: " << it2->autoindex << std::endl;
			std::cout << "     * upload_enable: " << it2->upload_enable << std::endl;
			std::cout << "     * upload_path: " << it2->upload_path << std::endl;
			++it2;
		}
	}
};

/**
* Get servers configuration
* @return servers configurations
*/
std::vector<Configuration::server> Configuration::getServers(void)
{
	return (_servers);
}