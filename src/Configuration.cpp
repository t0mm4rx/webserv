#include "Configuration.hpp"

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
* @param line_start the line where server definition starts
* @param line_end the line where server definition ends
*/
void Configuration::_parseServer(std::string source, size_t line_start, size_t line_end)
{
	server s;
	std::vector<std::string> line;
	char last;

	for (size_t n = line_start + 1; n < line_end; n++)
	{
		if (!isSkippable(source, n))
		{
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
	}
	_servers.push_back(s);
}

/**
* Prints an entire configuration to ensure parsing is good
*/
void Configuration::print(void)
{
	std::map<int, std::string>::iterator it;

	for (size_t i = 0; i < _servers.size(); i++)
	{
		std::cout << "- Server" << std::endl;
		std::cout << "   * server_name: " + _servers[i].name << std::endl;
		std::cout << "   * host: " + _servers[i].host << std::endl;
		std::cout << "   * port: " + std::to_string(_servers[i].port) << std::endl;
		std::cout << "   * client_max_body_size: " + std::to_string(_servers[i].client_max_body_size) << std::endl;
		it = _servers[i].error_pages.begin();
		while (it != _servers[i].error_pages.end())
		{
			std::cout << "   * error_page for " + std::to_string(it->first) + ": " + it->second << std::endl;
			++it;
		}
	}
};