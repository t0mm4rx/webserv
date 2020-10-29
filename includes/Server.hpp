/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rchallie <rchallie@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/10/21 15:24:51 by rchallie          #+#    #+#             */
/*   Updated: 2020/10/28 16:43:12 by rchallie         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
# define SERVER_HPP

#include "SocketManager.hpp"
#include "HeadersBlock.hpp"
#include <unistd.h>

class Server
{
    private:
        SocketManager<Socket>   _sm;

        int         waitConnection(fd_set *working_set, int max_sd);
        int         acceptConnection(int sd, int max_sd, fd_set *master_set, SocketManager<SubSocket> &sub_sm);
        int         receiveConnection(int sd, char *buffer, int buffer_size);
        int         closeConnection(int sd, int max_sd, fd_set *master_set);
        std::string getServerName(const HeadersBlock& hb);

    public:
        Server(SocketManager<Socket> sm);
        Server(const Server& copy);
        ~Server();
        Server &operator=(const Server& op);

        void loop();
};

#endif