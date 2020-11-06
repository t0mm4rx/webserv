#ifndef PARSING_HPP
# define PARSING_HPP

# include <string>
# define BUFFER_SIZE 256
# include <unistd.h>
# include <fcntl.h>
# include <iostream>
# include <cstdlib>
# include <vector>
# include <sstream>
# include <sys/stat.h>
# include <sys/time.h>
# include <sys/types.h>

static const char* server_properties[] = {
	"listen",
	"server_name",
	"error_page",
	"root",
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
	"client_max_body_size",
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
	0
};

size_t countLines(std::string source);
size_t getClosingBracket(std::string source, size_t line);
std::vector<std::string> parseProperty(std::string source, size_t line, std::string object);
std::vector<std::string> splitWhitespace(std::string source);
bool isPropertyNameValid(std::string name, const char **valid_names);
std::string readFile(std::string file);
std::vector<unsigned char> readBinaryFile(std::string file);
std::string itoa(int n);
std::string getLine(std::string, size_t line);
bool isSkippable(std::string source, size_t line);
bool endsWithOpenBracket(std::string source, size_t line);
size_t uIntegerParam(std::string param, size_t line);
bool boolParam(std::string param, size_t line);
bool isMethodValid(std::string method);
std::string uIntegerToString(size_t value);
std::string replace(std::string source, std::string to_replace, std::string new_value);
int pathType(std::string path, time_t *file_date);

class ParsingException : public std::exception
{
	private:
		std::string _msg;
	public:
		ParsingException(int line = 0, std::string msg = "Unable to parse the given config file.")
		: _msg("Line: " + uIntegerToString(line + 1) + ": " + msg)
		{};
		~ParsingException() throw()
		{};
		const char *what () const throw ()
		{
			return (_msg.c_str());
		};
};

#endif