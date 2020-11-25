/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rchallie <rchallie@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/10/21 15:24:51 by rchallie          #+#    #+#             */
/*   Updated: 2020/11/25 14:51:30 by rchallie         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
# define SERVER_HPP


#include "SocketManager.hpp"
#include "HeadersBlock.hpp"
#include "parsing.hpp"
#include <unistd.h>
#include <string>

class Server
{
	private:
		SocketManager<Socket *>   _sm;
		SocketManager<SubSocket *> _sub_sm;

		int			waitConnection(fd_set *read_set, fd_set *write_set, int max_sd);
		int         acceptConnection(int sd, int max_sd, fd_set *read_set, fd_set *write_set, SocketManager<SubSocket *>& sub_sm);
		int			receiveConnection(int sd, std::string& request);
		int			closeConnection(int sd, int max_sd, fd_set *read_set, fd_set *write_set);
		std::string getServerName(const HeadersBlock& hb);
		void		verifyDefaultServer();
		int			getChunk(int sd, std::vector<std::string>& request);


	public:
		Server();
		Server(SocketManager<Socket *> sm);
		Server(const Server& copy);
		~Server();
		Server &operator=(const Server& op);

		void loop();
		void closeServer();
};

#endif