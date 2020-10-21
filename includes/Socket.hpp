/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Socket.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rchallie <rchallie@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/10/20 16:34:20 by rchallie          #+#    #+#             */
/*   Updated: 2020/10/21 16:16:10 by rchallie         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SOCKET_HPP
# define SOCKET_HPP

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <fcntl.h> 

# include "WebServer.hpp"

#define MAX_CONNECTION 9999

class Socket
{
	private:
		int					_sd;
		int					_option_buffer;
		struct sockaddr_in	_address;

		void	createSocketDescriptor(void);
		void	setSocketOptions(void);
		void	setSocketNonBlocking(void);
		void	initAddress(int port);
		void	bindSocket(void);
		void	socketListener(void);
	
	public:
		Socket(const Config& config);
		Socket(const Socket& copy);
		virtual ~Socket();
		Socket &operator=(const Socket& op);

		int					getSocketDescriptor(void);
};

#endif