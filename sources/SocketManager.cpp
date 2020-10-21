/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   SocketManager.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rchallie <rchallie@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/10/20 22:45:10 by rchallie          #+#    #+#             */
/*   Updated: 2020/10/21 16:24:24 by rchallie         ###   ########.fr       */
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
:
    _sockets(copy._sockets)
{}

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
fd_set  SocketManager::getSDSet(void)
{
    fd_set sockets_fds_set;

    FD_ZERO(&sockets_fds_set);
    for (std::vector<Socket *>::iterator it = this->_sockets.begin(); it != this->_sockets.end(); it++)
        FD_SET((*it)->getSocketDescriptor(), &sockets_fds_set);
    return (sockets_fds_set);
}

int     SocketManager::getLastSD(void)
{
    return ((*(this->_sockets.end() - 1))->getSocketDescriptor());
}

bool    SocketManager::hasSD(int socket_descriptor)
{
    std::vector<Socket *>::iterator it;

    for (it = this->_sockets.begin(); it != this->_sockets.end(); it ++)
    {
        DEBUG("TEST = " << (*it)->getSocketDescriptor() << " == " << socket_descriptor)
        if ((*it)->getSocketDescriptor() == socket_descriptor)
            return (true);
    }
    return (false);
}
