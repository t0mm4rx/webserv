/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   SubSocket.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rchallie <rchallie@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/10/24 11:51:19 by rchallie          #+#    #+#             */
/*   Updated: 2020/11/22 21:15:59 by rchallie         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SUBSOCKET_HPP
# define SUBSOCKET_HPP

#include "Socket.hpp"

class SubSocket : public Socket
{
	private:
		Socket      *_parent_socket;
		std::string _client_ip;
		bool		_information_received;
		std::string _request;

	public:
		SubSocket(Socket *parent, std::string client_ip, int sd);
		SubSocket(const SubSocket& copy);
		virtual ~SubSocket();
		SubSocket &operator=(const Socket& op);

		Socket *getParent();
		std::string & getRequest();
		std::string getClientIp();
		bool informationReceived();
		void setReceived(bool val);
};

#endif