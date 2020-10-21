#include "parsing.hpp"

/**
* Get the string inner the curly braces pair starting at given line.
*
* @throw ParsingException if the brackets are not closed
* @param source the string to process
* @param line the line where curly brackets opens
* @return the line where the curly brackets ends
* @example "server {\na\nb\nc\n}" with line 0 will return line 4
*/
size_t getClosingBracket(std::string source, size_t line)
{
	size_t n;
	size_t size;
	size_t open_brackets;

	open_brackets = 0;
	if (getLine(source, line)[getLine(source, line).size() - 1] != '{')
		throw ParsingException(line, "Expected '{'.");
	n = line + 1;
	size = countLines(source);
	while (n < size)
	{
		if (!isSkippable(source, n) && endsWithOpenBracket(source, n))
			++open_brackets;
		if (!isSkippable(source, n) && getLine(source, n) == "}")
		{
			if (open_brackets == 0)
				return (n);
			--open_brackets;
		}
		++n;
	}
	throw ParsingException(line, "Expected '}'.");
	return (-1);
}

/**
* Does the given line in source has a '{' as last char
* @param source the config string
* @param line the line to check
* @return wether the lines ends with '{'
*/
bool endsWithOpenBracket(std::string source, size_t line)
{
	std::vector<std::string> splits;

	splits = splitWhitespace(getLine(source, line));
	if (splits.size() > 0)
	{
		if (splits[splits.size() - 1] == "{")
			return (true);
	}
	return (false);
}

/**
* Reads a file into a std::string
* @throw ParsingException if the file does not exists
* @param file the file to read
* @return a string containing the file content
*/
std::string readFile(std::string file)
{
	char buffer[BUFFER_SIZE + 1] = {0};
	int fd;
	int i;
	std::string result;

	fd = open(file.c_str(), O_RDONLY);
	if (fd < -1)
		throw ParsingException(0, "The file " + file + " does not exists.");
	while (read(fd, buffer, BUFFER_SIZE) > 0)
	{
		result += buffer;
		i = 0;
		while (i < BUFFER_SIZE)
			buffer[i++] = 0;
	}
	close(fd);
	return (result);
}

/**
* Get the n-th line of source, without whitespaces before and after
* @param source the config string
* @param n the wanted line
* @return the n-th line
*/
std::string getLine(std::string source, size_t n)
{
	size_t i;
	size_t j;
	size_t line_count;

	if (n >= countLines(source))
		return (std::string());
	line_count = 0;
	i = 0;
	while (line_count < n)
	{
		if (source[i++] == '\n')
			++line_count;
	}
	while (std::isspace(source[i]) && source[i] != '\n')
		++i;
	j = 0;
	while (source[i + j] && source[i + j] != '\n')
		++j;
	while (j > 0 && std::isspace(source[i + j - 1]))
		--j;
	return (std::string(source, i, j));
}

/**
* Count the number of lines in a string.
* @param source the config string
* @return the number of lines
*/
size_t countLines(std::string source)
{
	size_t i;
	size_t lines;

	i = 0;
	lines = 1;
	while (source[i])
	{
		if (source[i++] == '\n')
			++lines;
	}
	return (lines);
}

/**
* Check if a server or route property is well formated, valid, and then splits it in args
* @param source the config string
* @param line the line to parse
* @param object should be either "server" or "route"
* @return a vector of values, first is the key
* @throw ParsingException if the property isn't valid
* @example "listen 80 localhost;" will return a vector of 3 -> "listen", "80" and "localhost"
*/
std::vector<std::string> parseProperty(std::string source, size_t line, std::string object)
{
	std::vector<std::string> result;
	std::string l;

	l = getLine(source, line);
	if (l[l.size() - 1] != ';')
		throw ParsingException(line, "Expected ';'.");
	l = std::string(l, 0, l.size() - 1);
	result = splitWhitespace(l);
	if (result.size() <= 1)
		throw ParsingException(line, "Properties should have at least one value.");
	if (!isPropertyNameValid(result[0], object == "server" ? server_properties : route_properties))
		throw ParsingException(line, "'" + result[0] + "' is not a valid property for " + object + ".");
	return (result);
}

/**
* Splits a string by whitespaces
* @param str the string to split
* @return a vector of strings
*/
std::vector<std::string> splitWhitespace(std::string str)
{
	std::vector<std::string> res;
	size_t i;
	size_t j;

	i = 0;
	j = 0;
	while (str[i])
	{
		if (std::isspace(str[i]))
		{
			if (i == j)
				++j;
			else
			{
				res.push_back(std::string(str, j, i - j));
				j = i + 1;
			}
		}
		++i;
	}
	if (i != j)
		res.push_back(std::string(str, j, i - j));
	return (res);
}

/**
* Check if the property name is inside the given properties name list
* @param name the property name
* @param valid_propeties the list of valid properties name
* @return wether the name is valid or not
* @example "listen" in {"server_name", "listen", 0} will return true
* @example "hello" in {"server_name", "listen", 0} will return false
*/
bool isPropertyNameValid(std::string name, const char **valid_names)
{
	size_t i;

	i = 0;
	while (valid_names[i])
	{
		if (name == valid_names[i])
			return (true);
		++i;
	}
	return (false);
}

/**
* Check if the line is skippable (empty line or starting with #)
* @param source the config string
* @param line the line to check
* @return wether we want to skip the line or not
*/
bool isSkippable(std::string source, size_t line)
{
	std::string l;

	l = getLine(source, line);
	return (splitWhitespace(l).size() == 0 || l.size() == 0 || l[0] == '#');
}

/**
* Converts a string to an integer, check if it's positive.
* @param param the string to parse
* @param line the line where the param occurs
* @return the integer representation of the param
* @throw ParsingExecption if the string doesn't represent a positive integer
*/
size_t uIntegerParam(std::string param, size_t line)
{
	size_t value;

	try
	{
		value = std::stoi(param);
	}
	catch (const std::exception &e)
	{
		throw ParsingException(line, "'" + param + "' is not a positive integer.");
	}
	if (value < 0)
		throw ParsingException(line, "'" + param + "' is not a positive integer.");
	return (value);
}

/**
* Check if a given method exists
* @param method the method to check
* @return wether the method given is valid or not
*/
bool isMethodValid(std::string method)
{
	size_t i;

	i = 0;
	while (methods[i])
	{
		if (methods[i] == method)
			return (true);
		++i;
	}
	return (false);
}

/**
* Converts a "yes"/"no" string into a bool
* @param param the param to convert
* @param line the line where the param occurs
* @throw ParsingException if the param isn't "yes" or "no"
* @return the boolean value of the string
*/
bool boolParam(std::string param, size_t line)
{
	if (param == "yes")
		return (true);
	else if (param == "no")
		return (false);
	else
		throw ParsingException(line, "Boolean parameter should be \"yes\" or \"no\".");
}