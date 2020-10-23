/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   WebServ.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rchallie <rchallie@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/10/20 16:17:48 by rchallie          #+#    #+#             */
/*   Updated: 2020/10/22 00:12:03 by rchallie         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/WebServ.hpp"
#include "../includes/Socket.hpp"
#include "../includes/SocketManager.hpp"
#include "../includes/Server.hpp"
#include "../includes/Configuration.hpp"
#include "../includes/RequestInterpretor.hpp"

int main(int argc, char **argv)
{
    char *path;

    if (argc != 2)
    {
        outError("Please use : ./WebServ <path>");
        return (1);
    }
    path = argv[1];
    
    DEBUG("")
   
    std::vector<Socket *> socket_list;
    try
    {
        Configuration test = Configuration(path);
		    test.print();
        for (int i = 0; i < (int)test.getServers().size(); i++)
            socket_list.push_back(new Socket(test.getServers()[i]));
        SocketManager sm;

        for (int i = 0; i < (int)socket_list.size(); i++)
            sm.registerSocket(*(socket_list.begin() + i));

        Server server(sm);
        
        server.loop();
    }
    catch(const std::exception& e)
    {
        throwError(e);
    }
    return (0);
}

int treat(int sd, char *buffer)
{
	Configuration::server s;
	Configuration::location l;
	Configuration::location l1;
	Configuration::location l2;

	l.root = "/Users/tom/Documents/www/";
	l.index = "index.html";
	l.autoindex = false;
	l.name = "/";

	l1.name = "/wordpress";
	l1.root = "/Users/tom/Documents/www/";
	l1.index = "a";
	l2.name = "/upload";
	s.locations.push_back(l);
	s.locations.push_back(l1);
	s.locations.push_back(l2);
	std::string response = RequestInterpretor(std::string(buffer), s).getResponse();
    int rc = send(sd, response.c_str(), response.size(), 0);
    if (rc < 0)
    {
        DEBUG("  send() failed");
        return (-1);
    }
    return (0);
}