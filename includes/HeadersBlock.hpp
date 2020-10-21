/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HeadersBlock.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rchallie <rchallie@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/10/22 00:29:54 by rchallie          #+#    #+#             */
/*   Updated: 2020/10/22 01:11:38 by rchallie         ###   ########.fr       */
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
            std::string _filed_value;
        };

    private:
        struct request_line                 _request_line;
        struct status_line                  _status_line;
        std::vector<struct header_field>    _header_fields;

        void    getLines(std::string msg, std::vector<std::string> *lines);

    public:
        HeadersBlock(const std::string & block);
        HeadersBlock(const HeadersBlock& copy);
        ~HeadersBlock();
        HeadersBlock &operator=(const HeadersBlock& op);
};

#endif