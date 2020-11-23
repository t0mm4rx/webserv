/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   SubSocket.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rchallie <rchallie@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/10/24 11:56:20 by rchallie          #+#    #+#             */
/*   Updated: 2020/11/22 21:33:06 by rchallie         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/SubSocket.hpp"

SubSocket::SubSocket(Socket *parent, std::string client_ip, int sd)
:
	Socket(sd),
	_parent_socket(parent),
	_client_ip(client_ip),
	_information_received(false),
	_request()
{}

SubSocket::SubSocket(const SubSocket& copy)
:
	Socket(copy._sd),
	_parent_socket(copy._parent_socket),
	_client_ip(copy._client_ip),
	_information_received(copy._information_received),
	_request(copy._request)
{}

SubSocket::~SubSocket() {}
SubSocket &SubSocket::operator=(const Socket& op) {(void)op; return (*this);}

Socket *SubSocket::getParent()
{
	return (this->_parent_socket);
}

std::string &SubSocket::getRequest()
{
	return (this->_request);
}

std::string SubSocket::getClientIp()
{
	return (this->_client_ip);
}

bool SubSocket::informationReceived()
{
	return (this->_information_received);
}

void SubSocket::setReceived(bool val)
{
	this->_information_received = val;
	if (val == true)
		Log("Request received from : " + itoa(this->_sd));
}