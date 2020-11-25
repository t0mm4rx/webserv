/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HeadersBlock.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rchallie <rchallie@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/10/22 00:29:54 by rchallie          #+#    #+#             */
/*   Updated: 2020/11/24 13:11:10 by rchallie         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HEADERSBLOCK_HPP
# define HEADERSBLOCK_HPP

#include <string>
#include <vector>
#include "parsing.hpp"

class HeadersBlock
{
	public:
		struct request_line
		{
			std::string _method;
			std::string _request_target;
			std::string _http_version;
		};
		
		struct status_line
		{
			std::string _http_version;
			int         _status_code;
			std::string _reason_phrase;
		};

		struct header_field
		{
			std::string _field_name;
			std::string _field_value;
		};

	private:
		struct request_line                 _request_line;
		struct status_line                  _status_line;
		std::vector<struct header_field>    _header_fields;
		bool                                _is_request;
		std::string                         _client_ip;
		std::string                         _content;
		std::string							_raw_request;

		void    getLines(std::string msg, std::vector<std::string> *lines);
		void    getRequestLine(std::vector<std::string> lines);
		void    getStatusLine(std::vector<std::string> lines);
		int     getHeaderFileds(std::vector<std::string> lines);

	public:
		HeadersBlock(const std::string & request, const std::string & client_ip, int content_type);
		HeadersBlock(const HeadersBlock& copy);
		~HeadersBlock();
		HeadersBlock &operator=(const HeadersBlock& op);

		bool isRequest(void) const;
		void pushContent(std::string buffer);

		struct request_line getRequestLine(void) const;
		struct status_line getStatusLine(void) const;
		std::vector<struct header_field> getHeaderFields(void) const;
		std::string getContent(void) const;
		std::string getClientIP(void) const;
		std::string getPlainRequest(void) const;
};

std::ostream &operator<<(std::ostream &out, const HeadersBlock &hb);

#endif