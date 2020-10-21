/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   SocketManager.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rchallie <rchallie@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/10/20 22:45:21 by rchallie          #+#    #+#             */
/*   Updated: 2020/10/21 16:08:13 by rchallie         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SOCKETMANAGER_HPP
# define SOCKETMANAGER_HPP

#include <vector>
#include   <sys/time.h>
#include "Socket.hpp"

class SocketManager
{
    private:
        std::vector<Socket *>   _sockets;

    public:
        SocketManager();
        SocketManager(const SocketManager& copy);
        ~SocketManager();
        SocketManager &operator=(const SocketManager& op);

        void    registerSocket(Socket *socket);

        fd_set  getSDSet(void);
        int     getLastSD(void);

        bool    hasSD(int socket_descriptor);
};

#endif