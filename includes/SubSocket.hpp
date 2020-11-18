/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   SubSocket.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rchallie <rchallie@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/10/24 11:51:19 by rchallie          #+#    #+#             */
/*   Updated: 2020/10/28 17:19:03 by rchallie         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SUBSOCKET_HPP
# define SUBSOCKET_HPP

#include "Socket.hpp"

class SubSocket : public Socket
{
	private:
		Socket      _parent_socket;
		std::string _client_ip;

	public:
		SubSocket(Socket& parent, std::string client_ip, int sd);
		SubSocket(const SubSocket& copy);
		~SubSocket();
		SubSocket &operator=(const Socket& op);

		Socket &getParent();
		std::string getClientIp();
};

#endif