/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Socket.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rchallie <rchallie@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/10/20 16:34:20 by rchallie          #+#    #+#             */
/*   Updated: 2020/11/03 16:57:45 by rchallie         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SOCKET_HPP
# define SOCKET_HPP

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <fcntl.h>

# include "WebServ.hpp"
# include "Configuration.hpp"

#define MAX_CONNECTION 9999

#ifdef __APPLE__
# define MSG_NOSIGNAL 0
#endif

class Socket
{
	protected:
		int								_sd;

	private:
		int								_option_buffer;
		struct sockaddr_in				_address;
		struct Configuration::server	_server_config;

		void	createSocketDescriptor(void);
		void	setSocketOptions(void);
		void	setSocketNonBlocking(void);
		void	initAddress(int port);
		void	bindSocket(void);
		void	socketListener(void);
	
	public:
		Socket(const struct Configuration::server& server);
		Socket(int sd, const struct Configuration::server& server);
		Socket(int sd);
		Socket(const Socket& copy);
		virtual ~Socket();
		Socket &operator=(const Socket& op);

		int								getSocketDescriptor(void);
		struct Configuration::server	getServerConfiguration(void);
		void							setToDefault(void);
};

#endif