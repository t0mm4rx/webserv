/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   SocketManager.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rchallie <rchallie@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/10/20 22:45:10 by rchallie          #+#    #+#             */
/*   Updated: 2020/10/21 20:31:21 by rchallie         ###   ########.fr       */
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

/**
 *  @brief Add a socket to the socket manager.
 * 
 *  @param socket the socket.
 */
void SocketManager::registerSocket(Socket *socket)
{
    this->_sockets.push_back(socket);
}

/**
 *  @return Give a fd_set containing the socket descriptors
 *  of all sockets.
 */
fd_set  SocketManager::getSDSet(void)
{
    fd_set sockets_fds_set;

    FD_ZERO(&sockets_fds_set);
    for (std::vector<Socket *>::iterator it = this->_sockets.begin(); it != this->_sockets.end(); it++)
        FD_SET((*it)->getSocketDescriptor(), &sockets_fds_set);
    return (sockets_fds_set);
}

/**
 *  @return Give the last socket descriptor of the socket
 *  manager.
 */
int     SocketManager::getLastSD(void)
{
    return ((*(this->_sockets.end() - 1))->getSocketDescriptor());
}

/**
 *  @brief Give a boolean that contain if the socket descriptor
 *  given in param is a socket descriptor of a socket of the
 *  socket manager.
 * 
 *  @param socket_descriptor the socket descriptor to check.
 *  @return true if the socket descriptor is one of the socket
 *      manager, false otherwise.
 */
bool    SocketManager::hasSD(int socket_descriptor)
{
    std::vector<Socket *>::iterator it;

    for (it = this->_sockets.begin(); it != this->_sockets.end(); it ++)
    {
        if ((*it)->getSocketDescriptor() == socket_descriptor)
            return (true);
    }
    return (false);
}
