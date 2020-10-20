/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   SocketManager.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rchallie <rchallie@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/10/20 22:45:10 by rchallie          #+#    #+#             */
/*   Updated: 2020/10/20 23:26:21 by rchallie         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/SocketManager.hpp"

//WIP
SocketManager::SocketManager()
:
    _sockets()
{}

//WIP
SocketManager::SocketManager(const SocketManager& copy)
{(void)copy;}

// WIP
SocketManager::~SocketManager()
{}

// WIP
SocketManager &SocketManager::operator=(const SocketManager& op)
{(void)op; return (*this); }

void SocketManager::registerSocket(Socket *socket)
{
    this->_sockets.push_back(socket);
}

//WIP
fd_set  SocketManager::getFDSet(void)
{
    fd_set sockets_fds_set;

    FD_ZERO(&sockets_fds_set);
    for (std::vector<Socket *>::iterator it = this->_sockets.begin(); it != this->_sockets.end(); it++)
        FD_SET((*it)->getSocketDescriptor(), &sockets_fds_set);
    return (sockets_fds_set);
}
