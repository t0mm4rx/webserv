/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   WebServ.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rchallie <rchallie@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/10/20 16:17:48 by rchallie          #+#    #+#             */
/*   Updated: 2020/11/26 17:58:32 by rchallie         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/WebServ.hpp"
#include "../includes/Socket.hpp"
#include "../includes/SocketManager.hpp"
#include "../includes/Server.hpp"
#include "../includes/Configuration.hpp"
#include "../includes/RequestInterpretor.hpp"
#include "../includes/HeadersBlock.hpp"

char **g_envp;

/* Variable on global to remove still reachable */
Server server;
SocketManager<Socket *> sm;
Configuration test;

int main(int argc, char **argv, char **env)
{
	Log("Start Server(s)...");
	char *path;

	g_envp = env;
	if (argc != 2)
	{
		outError("Please use : ./WebServ <path>");
		return (1);
	}
	path = argv[1];
	try
	{
		test = Configuration(path);
		#if DEBUG_ACTIVE == 1
		test.print();
		#endif
		for (int i = 0; i < (int)test.getServers().size(); i++)
		{
			Socket *exist = NULL;
			for (size_t j = 0; j < sm.getSockets().size(); j++)
				if (sm.getSockets()[j]->getServerConfiguration().port == test.getServers()[i].port)
					exist = sm.getSockets()[j];
			if (exist == NULL)
				sm.registerSocket(new Socket(test.getServers()[i]));
			else
				sm.registerSocket(new Socket(exist->getSocketDescriptor(), test.getServers()[i]));
			Log("Server created on port " + itoa(test.getServers()[i].port)
				+ " : " + itoa(sm.getSockets()[i]->getSocketDescriptor()));
		}
		server = Server(sm);
		sm.getSockets().clear();
		signal(SIGINT, endServer);
		server.loop();
	}
	catch(const std::exception& e)
	{
		throwError(e);
	}
	return (0);
}

int treat(int sd, HeadersBlock &header_block, Configuration::server server_conf)
{
	std::string response = RequestInterpretor(header_block, server_conf).getResponse();

	Log("Send response to : " + itoa(sd));
	int	rc = write(sd, response.c_str(), response.size());
	DEBUG("END SEND");
	if (rc <= 0)
	{
		Log("Error or connection close on : " + itoa(sd));
		return (-1);
	}
	return (0);
}

void endServer(int signal)
{
	(void)signal;
	server.closeServer();
	Log("Ending server...");
	exit(0);
}