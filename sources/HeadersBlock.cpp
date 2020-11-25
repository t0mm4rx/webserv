/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HeadersBlock.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rchallie <rchallie@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/10/22 00:50:31 by rchallie          #+#    #+#             */
/*   Updated: 2020/11/24 18:57:18 by rchallie         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/HeadersBlock.hpp"
#include "../includes/WebServ.hpp"

static std::string alpha = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
static std::string digit = "0123456789";

/**
 *  @brief Check if the string containing the
 *  reason phrase is well formated.
 * 
 *  @param token the reason phrase string.
 *  @return true if it's valid, false otherwise.
 */
static bool isValidReasonPhrase(std::string reason_phrase)
{
	if (reason_phrase.length() == 0)
		return (false);
	for (size_t i = 0; i < reason_phrase.length(); i++)
		if (reason_phrase[i] != '\t'
			&& reason_phrase[i] != ' '
			&& !((unsigned long)reason_phrase[i] >= (unsigned long)20
			&& (unsigned long)reason_phrase[i] <= (unsigned long)126))
		return (false);
	return (true);
}

/**
 *  @brief Check if the string containing the
 *  status code is well formated.
 * 
 *  @param token the status code string.
 *  @return true if it's valid, false otherwise.
 */
static bool isValidStatusCode(std::string status_code)
{
	if (status_code.length() != 3)
		return (false);
	for (size_t cur = 0; cur < status_code.length(); cur++)
		if (digit.find(status_code[cur]) == std::string::npos)
			return (false);
	return (true);
}

/**
 *  @brief Check if the string containing the
 *  http version is well formated.
 * 
 *  @param token the https version string.
 *  @return true if it's valid, false otherwise.
 */
static bool isValidHTTPVersion(std::string http_version)
{
	if (http_version.length() != 8)
		return (false);
	if (http_version[0] == 'H'
			&& http_version[1] == 'T' && http_version[2] == 'T'
			&& http_version[3] == 'P' && http_version[4] == '/'
			&& digit.find(http_version[5]) != std::string::npos
			&& http_version[6] == '.'
			&& digit.find(http_version[7]) != std::string::npos)
		return (true);
	return (false);
}

/**
 *  @brief Check if the string containing the
 *  method is well formated.
 * 
 *  @param token the method string.
 *  @return true if it's valid, false otherwise.
 */
static bool isValidToken(std::string token)
{
	std::string vchars = "!#$%&'*+-.^_`|~" + digit + alpha;

	if (token.length() == 0)
		return (false);
	for (size_t cur = 0; cur < token.length(); cur++)
		if (vchars.find(token[cur]) == std::string::npos)
			return (false);
	return (true);
}

/**
 *  @brief Split the block in a vector of line.
 * 
 *  @param msg the block to split.
 *  @param lines the vector. Can be a reference in the futur.
 */
void    HeadersBlock::getLines(std::string msg, std::vector<std::string> *lines)
{
	size_t start = 0;
	size_t end = msg.find("\n");
	while (end != std::string::npos)
	{
		(*lines).push_back(msg.substr(start, end - start));
		start = end + 1;
		end = msg.find("\n", start);
	}
	(*lines).push_back(msg.substr(start, end - start));
}

/**
 *  @brief Parse a request line. (start line)
 *  Like the it's a start line, the getted line is the
 *  first of the header.
 * 
 *  @param lines the lines of the header.
 */
void    HeadersBlock::getRequestLine(std::vector<std::string> lines)
{
	// Method
	this->_request_line._method = lines[0].substr(0, lines[0].find(' '));
	if (isValidToken(this->_request_line._method) == false)
		throw(throwMessage("Request line not well formated (method)."));
	lines[0].erase(0, this->_request_line._method.length() + 1);

	// Request Target
	this->_request_line._request_target = lines[0].substr(0, lines[0].find(' '));
	lines[0].erase(0, this->_request_line._request_target.length() + 1);

	// HTTP-version
	if (lines[0].find('\r') != std::string::npos)
		this->_request_line._http_version = lines[0].substr(0, lines[0].find('\r'));
	else if (lines[0].find('\n') != std::string::npos)
		this->_request_line._http_version = lines[0].substr(0, lines[0].find('\n'));
	else
		this->_request_line._http_version = lines[0].substr(0, lines[0].find(' '));
	if(isValidHTTPVersion(this->_request_line._http_version) == false)
		throw(throwMessage("Request line not well formated (http version) : " + this->_request_line._http_version));
	this->_is_request = true;
}

/**
 *  @brief Parse a status line. (start line)
 *  Like the it's a start line, the getted line is the
 *  first of the header.
 * 
 *  @param lines the lines of the header.
 */
void    HeadersBlock::getStatusLine(std::vector<std::string> lines)
{
	std::string first_word = lines[0].substr(0, lines[0].find(" "));
	std::string tmp;

	// HTTP-version
	this->_status_line._http_version = first_word;
	if (isValidHTTPVersion(this->_status_line._http_version) == false)
		throw(throwMessage("Status line not well formated (http_version)."));
	lines[0].erase(0, first_word.length() + 1);

	// Status code
	tmp = lines[0].substr(0, lines[0].find(" "));
	if (isValidStatusCode(tmp) == false)
		throw(throwMessage("Status line not well formated (status code)."));
	this->_status_line._status_code = std::atoi(tmp.c_str());
	lines[0].erase(0, tmp.length() + 1);
	
	// Reason phrase
	if (lines[0].find('\r') != std::string::npos)
		this->_request_line._http_version = lines[0].substr(0, lines[0].find('\r'));
	else if (lines[0].find('\n') != std::string::npos)
		this->_request_line._http_version = lines[0].substr(0, lines[0].find('\n'));
	else
		this->_request_line._http_version = lines[0].substr(0, lines[0].find(' '));
	this->_status_line._reason_phrase = lines[0].substr(0, lines[0].find(' '));
	if (isValidReasonPhrase(this->_status_line._reason_phrase) == false)
		throw(throwMessage("Status line not well formated (reason phrase)."));
	
	this->_is_request = false;
}

/**
 *  @brief Parse a line from the headers block that
 *  isn't the first. Set this in a structure containing
 *  the field name and the field value.
 * 
 *  @param lines the lines of the header.
 */
int    HeadersBlock::getHeaderFileds(std::vector<std::string> lines)
{
	size_t i = 1;

	size_t lines_size = lines.size();
	for (i = 1; i < lines_size; i++)
	{
		struct header_field field;
		size_t posin;

		DEBUG("CHECKED = " << lines[i]);

		// Empty
		if (lines[i] == "\r" || lines[i].length() == 0)
			return (i);
		
		// Field name
		if ((posin = lines[i].find(':')) == std::string::npos) 
			throw(throwMessage("Header field name not well formated : " + lines[i]));
		field._field_name = lines[i].substr(0, posin);
		if (isValidToken(field._field_name) == false)
			throw(throwMessage("Header field name not well formated : " + lines[i]));
		lines[i].erase(0, posin + 1);
		
		// OWS
		if (lines[i][0] == ' ')
			lines[i].erase(0, 1);

		// Field value
		if (lines[i].find('\r') != std::string::npos)
			field._field_value = lines[i].substr(0, lines[i].find('\r'));
		else if (lines[i].find('\n') != std::string::npos)
			field._field_value = lines[i].substr(0, lines[i].find('\n'));
		else
			field._field_value = lines[i].substr(0, lines[i].length());
			
		this->_header_fields.push_back(field);
	}
	return (i);
}

HeadersBlock::HeadersBlock(const std::string & request, const std::string & client_ip, int content_type)
:
	_is_request(false),
	_client_ip(client_ip),
	_content(),
	_raw_request(request)
{
	std::vector<std::string> block_lines;
	
	getLines(request, &block_lines);
	try
	{
		if (block_lines.size() < 1)
			throw(throwMessage("Not valid header (size to low)"));
		size_t pos = block_lines[0].find(" ");
		std::string first_word = block_lines[0].substr(0, block_lines[0].find(" "));
		if (pos != std::string::npos)
		{
			int methods_number = 0;
			int i;
			while(methods[methods_number++]);

			for (i = 0; i < methods_number && methods[i]; i++)
			{
				if (first_word == methods[i])
				{
					this->getRequestLine(block_lines);
					break;
				}
			}
			if (methods[i] == NULL)
				this->getStatusLine(block_lines);
		}
		size_t end_headers = this->getHeaderFileds(block_lines);
		while (42)
		{
			if (end_headers < block_lines.size() && block_lines[end_headers] == "\r")
				end_headers++;
			else
				break;
		}
		if (content_type != 2)	/* Non chunked transfer */
			for (size_t i = end_headers; i < block_lines.size(); i++)
				this->pushContent(block_lines[i]);
		else if (content_type == 2)
		{
			while (42)
			{
				size_t nbr_char = std::stoi(block_lines[end_headers].c_str(), 0, 16);
				end_headers++;
				if (nbr_char == 0)
					break;
				std::string new_line;
				for (size_t i = 0; i < nbr_char; i++)
				{
					size_t pos_cut = nbr_char;
					std::string line = block_lines[end_headers];
					size_t line_len = line.length();
					if (nbr_char > line_len - 1)
							pos_cut = line_len;
					new_line.append(line.substr(0, pos_cut));
					if (line_len - 1 < nbr_char - i)
					{
						new_line.append("\n");
						i++;
					}
					end_headers++;
					i += pos_cut;
				}
				if (block_lines[end_headers][0] == '\r')
				{
					new_line.append("\n");
					end_headers++;
				}
				this->pushContent(new_line);
			}
		}
	}
	catch (const std::exception& e)
	{
		throwError(e);
		exit(0);
		throw(throwMessage("Can't parse header."));
	}
}

HeadersBlock::HeadersBlock(const HeadersBlock& copy)
:
	_request_line(copy._request_line),
	_status_line(copy._status_line),
	_header_fields(copy._header_fields),
	_is_request(copy._is_request),
	_client_ip(copy._client_ip),
	_content(copy._content),
	_raw_request(copy._raw_request)
{}

HeadersBlock::~HeadersBlock()
{}

HeadersBlock &HeadersBlock::operator=(const HeadersBlock& op)
{
	if (&op == this)
		return (*this);
	_request_line = op._request_line;
	_status_line = op._status_line;
	_header_fields = op._header_fields;
	_is_request = op._is_request;
	_client_ip = op._client_ip;
	_content = op._content;
	_raw_request = op._raw_request;
	return (*this);
}

bool HeadersBlock::isRequest(void) const
{
	return (this->_is_request);
}

void HeadersBlock::pushContent(std::string buffer)
{
	this->_content += buffer;
}

struct HeadersBlock::request_line HeadersBlock::getRequestLine(void) const
{
	return (this->_request_line);   
}

struct HeadersBlock::status_line HeadersBlock::getStatusLine(void) const
{
	return (this->_status_line);
}

std::vector<struct HeadersBlock::header_field> HeadersBlock::getHeaderFields(void) const
{
	return (this->_header_fields);
}

std::string HeadersBlock::getContent(void) const
{
	return (this->_content);
}

std::ostream &operator<<(std::ostream &out, const HeadersBlock &hb)
{
	if (hb.isRequest())
	{
		out << "START LINE = ";
		out << hb.getRequestLine()._method << " | ";
		out << hb.getRequestLine()._request_target;
		out << " | " << hb.getRequestLine()._http_version << std::endl;
	}
	else
	{
		out << "START LINE = ";
		out << hb.getStatusLine()._http_version << " | ";
		out << hb.getStatusLine()._status_code;
		out << " | " << hb.getStatusLine()._reason_phrase << std::endl;
	}

	for (size_t i = 0; i < hb.getHeaderFields().size(); i++)
	{
		out << "HEADER_FIELD = ";
		out << hb.getHeaderFields()[i]._field_name << " | ";
		out<< hb.getHeaderFields()[i]._field_value << std::endl;
	}
	return (out);
}

/**
 * Get client IP
 * @return the string representation of the client IP
 */
std::string HeadersBlock::getClientIP(void) const
{
	return (_client_ip);
}

/**
 * Get the string plain request.
 * @return the string representation of the request received.
 */
std::string HeadersBlock::getPlainRequest(void) const
{
	return (_raw_request);
}