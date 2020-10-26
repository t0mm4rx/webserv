/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HeadersBlock.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rchallie <rchallie@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/10/22 00:50:31 by rchallie          #+#    #+#             */
/*   Updated: 2020/10/23 18:51:44 by rchallie         ###   ########.fr       */
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
 *  @param lines the vector.
 */
void    HeadersBlock::getLines(std::string msg, std::vector<std::string> *lines)
{
    size_t pos = 0;
    while ((pos = msg.find("\n")) != std::string::npos)
    {
        (*lines).push_back(msg.substr(0, pos));
        msg.erase(0, pos + 1);
    }
    (*lines).push_back(msg.substr(0, pos));
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
void    HeadersBlock::getHeaderFileds(std::vector<std::string> lines)
{
    for (size_t i = 1; i < lines.size(); i++)
    {
        struct header_field field;

        size_t posin;

        // Empty
        if (lines[i] == "\r" || lines[i].length() == 0)
            continue;
        
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
}

HeadersBlock::HeadersBlock(const std::string & block)
:
    _is_request(false)
{
    std::vector<std::string> lines;

    this->getLines(block, &lines);

    size_t pos = lines[0].find(" ");
    std::string first_word = lines[0].substr(0, lines[0].find(" "));

    try
    {
        if (pos != std::string::npos)
        {
            int methods_number = 0;
            int i;
            while(methods[methods_number++]);

            for (i = 0; i < methods_number && methods[i]; i ++)
            {
                if (first_word == methods[i])
                {
                    this->getRequestLine(lines);
                    break;
                }
            }
            if (methods[i] == NULL)
                this->getStatusLine(lines);
        }
    this->getHeaderFileds(lines);
    }
    catch (const std::exception& e)
    {
        throwError(e);
        throw(throwMessage("Can't parse header.")); 
    }
}

//WIP
HeadersBlock::HeadersBlock(const HeadersBlock& copy)
{(void)copy;}

//WIP
HeadersBlock::~HeadersBlock()
{}

//WIP
HeadersBlock &HeadersBlock::operator=(const HeadersBlock& op)
{(void)op; return (*this);}

bool HeadersBlock::isRequest(void) const
{
    return (this->_is_request);
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