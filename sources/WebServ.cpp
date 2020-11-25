/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   WebServ.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rchallie <rchallie@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/10/20 16:17:48 by rchallie          #+#    #+#             */
/*   Updated: 2020/11/12 15:23:12 by rchallie         ###   ########.fr       */
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

int main(int argc, char **argv, char **env)
{
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
		Configuration test = Configuration(path);
		#if DEBUG_ACTIVE == 1
		test.print();
		#endif
		SocketManager<Socket> sm;
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
		}
		Server server(sm);
		signal(SIGINT, endServer);
		server.loop();
	}
	catch(const std::exception& e)
	{
		throwError(e);
	}
	return (0);
}

int treat(int sd, fd_set working_set, HeadersBlock &header_block, Configuration::server server_conf)
{
	std::string response = RequestInterpretor(header_block, server_conf).getResponse();
	#if DEBUG_ACTIVE == 1
	std::cout << "HEADER = [\n";
	for (int i = 0; i < 100; i++)
		std::cout << response[i];
	std::cout << "] " << std::endl;
	#endif
	DEBUG("SEND");
	int rc = 0;
	if (FD_ISSET(sd, &working_set))
		rc = write(sd, response.c_str(), response.size());
	DEBUG("END SEND");
	if (rc < 0)
	{
		DEBUG("send() failed");
		return (-1);
	}
	return (0);
}

void endServer(int signal)
{
	(void)signal;
	std::cout << "\rEnding server..." << std::endl;
	exit(0);
}