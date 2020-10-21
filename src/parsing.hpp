#ifndef PARSING_HPP
# define PARSING_HPP

# include <string>
# define BUFFER_SIZE 256
# include <unistd.h>
# include <fcntl.h>
# include <iostream>
# include <stdlib.h>
# include <vector>

static const char* server_properties[] = {
	"listen",
	"server_name",
	"error_page",
	"client_max_body_size",
	0
};

static const char* route_properties[] = {
	"method",
	"root",
	"autoindex",
	"index",
	"cgi_extension",
	"cgi_path",
	"upload_enable",
	"upload_path",
	0
};

static const char* methods[] = {
	"GET",
	"HEAD",
	"POST",
	"PUT",
	"DELETE",
	"CONNECT",
	"OPTIONS",
	"TRACE",
	"PATCH",
	0
};

class ParsingException : public std::exception
{
	private:
		std::string _msg;
	public:
		ParsingException(int line = 0, std::string msg = "Unable to parse the given config file.")
		: _msg("Line " + std::to_string(line + 1) + ": " + msg)
		{};
		~ParsingException() throw()
		{};
		const char *what () const throw ()
		{
			return (_msg.c_str());
		};
};

size_t countLines(std::string source);
size_t getClosingBracket(std::string source, size_t line);
std::vector<std::string> parseProperty(std::string source, size_t line, std::string object);
std::vector<std::string> splitWhitespace(std::string source);
bool isPropertyNameValid(std::string name, const char **valid_names);
std::string readFile(std::string file);
std::string itoa(int n);
std::string getLine(std::string, size_t line);
bool isSkippable(std::string source, size_t line);
bool endsWithOpenBracket(std::string source, size_t line);
size_t uIntegerParam(std::string param, size_t line);
bool boolParam(std::string param, size_t line);
bool isMethodValid(std::string method);

#endif