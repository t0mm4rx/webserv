/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   SubSocket.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rchallie <rchallie@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/10/24 11:56:20 by rchallie          #+#    #+#             */
/*   Updated: 2020/10/24 19:48:27 by rchallie         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/SubSocket.hpp"

SubSocket::SubSocket(Socket& parent, int sd)
:
    Socket(sd),
    _parent_socket(parent)
{}

SubSocket::SubSocket(const SubSocket& copy) : Socket(copy._sd),_parent_socket(copy._parent_socket) {(void)copy;}
SubSocket::~SubSocket() {}
SubSocket &SubSocket::operator=(const Socket& op) {(void)op; return (*this);}

Socket &SubSocket::getParent()
{ return (this->_parent_socket); }
