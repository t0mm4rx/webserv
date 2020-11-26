/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Socket.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rchallie <rchallie@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/10/20 16:38:43 by rchallie          #+#    #+#             */
/*   Updated: 2020/11/26 17:05:18 by rchallie         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Socket.hpp"

/**
 *  @brief Create the socket and give is socket descriptor.
 *  The socket is under IPV4 protocol (PF_INET / AF_INET)
 *  and is a support of dialog guaranteeing the data
 *  integrity.
 * 
 * 	@throw if the socket can't be create.
 */
void Socket::createSocketDescriptor(void)
{
	DEBUG("Creation of socket socket descriptor...")
	if ((this->_sd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
		throw(
			throwMessageErrno("Creation of the socket descriptor of a socket")
		);
}

/**
 *  @brief Set the options socket.
 *  Set the socket to be at the Socket level and
 *  set that the address is an reusable local address,
 *  same for port.
 * 
 * 	@throw if the options can't be set to the socket.
 */
void Socket::setSocketOptions(void)
{
	DEBUG("Set socket options...")
	if (setsockopt(this->_sd, SOL_SOCKET, SO_REUSEADDR/* | SO_REUSEPORT*/,
		&this->_option_buffer, sizeof(this->_option_buffer)))
		throw(
			throwMessageErrno("Set option of a socket")
		);
}

/**
 * 	@brief Set the socket to be nonblocking socket.
 * 
 * 	@throw if the attributes can't be set to the socket.
 */
void Socket::setSocketNonBlocking(void)
{
	DEBUG("Set socket to nonblocking...")
	if (fcntl(this->_sd, F_SETFL, O_NONBLOCK) < 0)
		throw(throwMessageErrno("Socket to nonblocking"));
}

/**
 * 	@brief Initialisation of the address informations.
 * 	It's based on the same protocol than the socket,
 * 	the address is attached to any of the possibilities and
 * 	the port defined is the configuration file.
 * 
 * 	@param port the port to bin the addres.
 */
void Socket::initAddress(int port)
{
	DEBUG("Init Address...")
	this->_address.sin_family = AF_INET;
	this->_address.sin_addr.s_addr = INADDR_ANY; 
	this->_address.sin_port = htons( port ); 
}

/**
 * 	@brief Bind the socket to the address.
 * 
 * 	@throw if the socket can't be bind.
 */
void Socket::bindSocket(void)
{
	DEBUG("Bind socket...");
	if (bind(this->_sd, (struct sockaddr *)&this->_address,
		sizeof(this->_address)) < 0)
		throw(
			throwMessageErrno("Bind socket")
		);
}

/**
 * 	@brief Set the socket to be a listener.
 * 	That permetted that it can use accept() the
 * 	incoming connections.
 * 	
 * @throw if the socket can't be set to listener.
 */
void Socket::socketListener(void)
{
	DEBUG("Socket Listener...");
	if (listen(this->_sd, MAX_CONNECTION) < 0)
		throw(throwMessageErrno("Set socket to listener"));
}

// PUBLIC

/**
 *  @brief Create a new socket using "config".
 *
 *  @param config the config of the socket.
 */
Socket::Socket(const struct Configuration::server& server)
:
	_sd(-1), _option_buffer() ,_server_config(server)
{
	try
	{
		this->createSocketDescriptor();
		this->setSocketOptions();
		this->setSocketNonBlocking();
		this->initAddress(server.port);
		this->bindSocket();
		this->socketListener();
	}
	catch(const std::exception& e)
	{
		throwError(e);
		throw(throwMessage("Stop program"));
	}
}

Socket::Socket(int sd, const struct Configuration::server& server)
:
	_sd(sd),
	_server_config(server)
{}

Socket::Socket(int sd)
:
	_sd(sd)
{}

/**
 * 	@brief Copy another socket.
 * 
 * 	@param copy the socket to copy.
 */
Socket::Socket(const Socket& copy)
:
	_sd(copy._sd),
	_option_buffer(copy._option_buffer),
	_address(copy._address),
	_server_config(copy._server_config)
{}

Socket::~Socket()
{}

Socket &Socket::operator=(const Socket& op)
{
	if (&op == this)
		return (*this);
	this->_sd = op._sd;
	this->_option_buffer = op._option_buffer;
	this->_address = op._address;
	this->_server_config = op._server_config;
	return (*this);
}

/**
 * 	@return the socket descriptor of the socket.
 */
int Socket::getSocketDescriptor(void)
{
	return (this->_sd);
}

struct Configuration::server	Socket::getServerConfiguration(void)
{
	return (this->_server_config);
}

void							Socket::setToDefault(void)
{
	this->_server_config.names.push_back("default_server");
}
