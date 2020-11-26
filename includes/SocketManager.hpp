/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   SocketManager.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rchallie <rchallie@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/10/20 22:45:21 by rchallie          #+#    #+#             */
/*   Updated: 2020/11/26 17:52:20 by rchallie         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SOCKETMANAGER_HPP
# define SOCKETMANAGER_HPP

#include <vector>
#include   <sys/time.h>
#include "Socket.hpp"
#include "SubSocket.hpp"

template <class T = Socket>
class SocketManager
{
	private:
		std::vector<T>   _sockets;

	public:
		//WIP
		SocketManager()
		:
			_sockets()
		{}
		
		//WIP
		SocketManager(const SocketManager& copy)
		:
			_sockets(copy._sockets)
		{}
		
		//WIP
		~SocketManager() {}
		
		//WIP
		SocketManager &operator=(const SocketManager& op)
		{
			if (&op == this)
				return (*this);
			this->_sockets = op._sockets;
			return (*this);
		}

		/**
		 *  @brief Add a socket to the socket manager.
		 * 
		 *  @param socket the socket.
		 */
		void registerSocket(T socket)
		{
			this->_sockets.push_back(socket);
		}

		fd_set  getSDSet(void)
		{
			fd_set sockets_fds_set;

			FD_ZERO(&sockets_fds_set);
			for (typename std::vector<T>::iterator it = this->_sockets.begin(); it != this->_sockets.end(); it++)
				FD_SET((*it)->getSocketDescriptor(), &sockets_fds_set);
			return (sockets_fds_set);
		}

		int     getLastSD(void)
		{
			return ((*(this->_sockets.end() - 1))->getSocketDescriptor());
		}

		T   &getBySD(int sd)
		{
			for (typename std::vector<T>::iterator it = this->_sockets.begin(); it != this->_sockets.end(); it++)
			{
				if ((*it)->getSocketDescriptor() == sd)
					return (*it);
			}
			throw(throwMessage("SD not found."));
		}

		T   &getBySDandHost(int sd, std::string host)
		{
			for (typename std::vector<T>::iterator it = this->_sockets.begin(); it != this->_sockets.end(); it++)
				if ((*it)->getSocketDescriptor() == sd)
					for (size_t j = 0; j < (*it)->getServerConfiguration().names.size(); j++)
						if ((*it)->getServerConfiguration().names[j] == host)
							return (*it);
			for (typename std::vector<T>::iterator it = this->_sockets.begin(); it != this->_sockets.end(); it++)
				if ((*it)->getSocketDescriptor() == sd)
					for (size_t j = 0; j < (*it)->getServerConfiguration().names.size(); j++)
						if ((*it)->getServerConfiguration().names[j] == "default_server")
							return (*it);
			return (this->getBySD(sd));
		}

		std::vector<T>  &getSockets(void)
		{
			return (this->_sockets);
		}

		/**
		 *  @brief Give a boolean that contain if the socket descriptor
		 *  given in param is a socket descriptor of a socket of the
		 *  socket manager.
		 * 
		 *  @param socket_descriptor the socket descriptor to check.
		 *  @return true if the socket descriptor is one of the socket
		 *      manager, false otherwise.
		 */
		bool    hasSD(int socket_descriptor)
		{
			typename std::vector<T>::iterator it = this->_sockets.begin();

			for (; it != this->_sockets.end(); it ++)
			{
				if ((*it)->getSocketDescriptor() == socket_descriptor)
					return (true);
			}
			return (false);
		}
};

#endif