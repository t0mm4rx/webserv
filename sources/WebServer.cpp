/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   WebServer.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rchallie <rchallie@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/10/20 16:17:48 by rchallie          #+#    #+#             */
/*   Updated: 2020/10/20 23:42:01 by rchallie         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/WebServer.hpp"
#include "../includes/Socket.hpp"
#include "../includes/SocketManager.hpp"

int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;
    DEBUG("")
    Config config(2020);
    Socket *plop = new Socket(config);
    SocketManager sm;

    sm.registerSocket(plop);

    fd_set sets = sm.getFDSet();

    struct timeval      timeout;
       timeout.tv_sec  = 3 * 60;
   timeout.tv_usec = 0;
    
    delete plop;
    return (0);
}