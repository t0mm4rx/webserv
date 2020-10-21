/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HeadersBlock.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rchallie <rchallie@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/10/22 00:50:31 by rchallie          #+#    #+#             */
/*   Updated: 2020/10/22 01:50:51 by rchallie         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/HeadersBlock.hpp"

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

HeadersBlock::HeadersBlock(const std::string & block)
{
    std::string msg = block;

    std::vector<std::string> lines;

    this->getLines(msg, &lines);

    for (int i = 0; i < (int)lines.size(); i++)
        std::cout << "Line [" << i << "] = " << lines[i] << std::endl;

    size_t pos = lines[0].find(" ");
    std::string first_word = lines[0].substr(0, lines[0].find(" "));

    std::cout << "FIRST WORD = " << first_word << std::endl;
    if (pos != std::string::npos)
    {
        int methods_number = 0;
        int i;
        while(methods[methods_number++]);

        for (i = 0; i < methods_number && methods[i]; i ++)
        {
            if (first_word == methods[i])
            {
                std::cout << "GET REQUEST LINE" << std::endl;
                break;
            }
        }
        if (methods[i] == NULL)
        {
            if (first_word.length() >= 8 && first_word[0] == 'H'
            && first_word[1] == 'T' && first_word[2] == 'T'
            && first_word[3] == 'P' && first_word[4] == '/'
            && (first_word[5] - '0' >= 0 && first_word[5] - '0' <= 9)
            && first_word[6] == '.'
            && (first_word[7] - '0' >= 0 && first_word[7] - '0' <= 9))
                std::cout << "GET STATUS LINE" << std::endl;
        }
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