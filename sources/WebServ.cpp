/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   WebServ.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rchallie <rchallie@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/10/20 16:17:48 by rchallie          #+#    #+#             */
/*   Updated: 2020/10/21 21:34:44 by rchallie         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/WebServ.hpp"
#include "../includes/Socket.hpp"
#include "../includes/SocketManager.hpp"
#include "../includes/Server.hpp"
#include "../includes/Configuration.hpp"

int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;
    DEBUG("")
   
    Socket *plop = NULL;
    Socket *plop2 = NULL;
    try
    {
        Configuration test = Configuration("./confs/test.conf");
		test.print();
		std::cout << test.getServers().size() << std::endl;
        Config config(test.getServers()[0].port);
        Config config2(test.getServers()[1].port);
        plop = new Socket(config);
        plop2 = new Socket(config2);
        SocketManager sm;

        sm.registerSocket(plop);
        sm.registerSocket(plop2);

        Server server(sm);
        
        server.loop();
    }
    catch(const std::exception& e)
    {
        throwError(e);
    }
    delete plop;
    delete plop2;
    return (0);
}

//Temptorary
int treat(int sd, char *buffer)
{
    (void)buffer;
    DEBUG("TREATMENT");
    DEBUG("BUFFER :\n" << buffer);

    char bufferrtn[] = "HTTP/1.1 200 OK\nContent-Type: text/html\nContent-Length: 14\n\n<h1>Hello</h1>";      
    int rc = send(sd, bufferrtn, strlen(bufferrtn), 0);
    if (rc < 0)
    {
        DEBUG("  send() failed");
        return (-1);
    }
    return (0);
}