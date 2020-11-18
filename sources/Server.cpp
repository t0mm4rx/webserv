/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rchallie <rchallie@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/10/21 15:25:08 by rchallie          #+#    #+#             */
/*   Updated: 2020/11/16 17:22:12 by rchallie         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Server.hpp"

//WIP
Server::Server(SocketManager<Socket> sm)
:
	_sm(sm)
{}

//WIP
Server::Server(const Server& copy)
{(void)copy;}

//WIP
Server::~Server()
{}

//WIP
Server &Server::operator=(const Server& op)
{(void)op; return (*this); }


/**
 *  @brief Wait that one of the listener sockets have a
 *  connection.
 * 
 *  @param working_set the socket descriptor set of treatment.
 *  @param max_sd the total of socket descriptor.
 *  @return the number of sockets that are ready.
 */
int Server::waitConnection(fd_set *working_set, int max_sd)
{
	int socket_ready;

	if ((socket_ready = select(max_sd + 1,working_set,
		NULL, NULL, NULL)) < 0)
		throw(throwMessageErrno("Wait connection"));
	return (socket_ready);
}

static std::string ft_inet_ntoa(struct in_addr in)
{
	std::stringstream buffer;

	unsigned char *bytes = (unsigned char *) &in;
	for (int cur_bytes = 0; cur_bytes < 4; cur_bytes++)
	{
		buffer << (int)bytes[cur_bytes];
		if (cur_bytes != 3)
			buffer << '.';
	}
	DEBUG("INET NTOA = " << buffer.str())
	return (buffer.str());
}

/**
 *  @brief Accept all connections in the queue of the
 *  socket descriptor, add them to the total of connected
 *  sockets descriptors (not to listener).
 * 
 *  @param sd the socket descriptor where accept.
 *  @param max_sd the total of socket descriptor.
 *  @param master_set the set of all sockets descriptors
 *      (listener + opened socket with clients).
 *  @return new number of maximum sockets descriptor.
 */
int Server::acceptConnection(int sd, int max_sd, fd_set *master_set, SocketManager<SubSocket>& sub_sm)
{
	int new_sd = 0;

	DEBUG("Readable socket")

	do
	{
		sockaddr_in client;
		socklen_t size = sizeof(client);
		if ((new_sd = accept(sd, (sockaddr *)&client, &size)) < 0)
		{
			if (errno != EWOULDBLOCK)
				throw(throwMessageErrno("TO REPLACE BY ERROR PAGE : accept() failed"));
			break;
		}
		
		DEBUG("New connection added")
		sub_sm.registerSocket(new SubSocket(this->_sm.getBySD(sd), ft_inet_ntoa(client.sin_addr), new_sd));
		FD_SET(new_sd, master_set);
		if (new_sd > max_sd)
			max_sd = new_sd;
	} while (new_sd != -1);
	
	return (max_sd);
}

/**
 *  @brief Set the message get from the socket into
 *  the buffer.
 * 
 *  @param sd the socket.
 *  @param buffer the buffer to stock the message.
 *  @param buffer_size the size of the buffer.
 *  @return -1 if and error appear, 0 otherwise.
 */
int Server::receiveConnection(int sd, std::vector<std::string>& request)
{
	int rc = 0;
	char buffer_recv[1];
	std::string line;
	bool headers_ended = false;
	int content_type = 0;
	int content_len = 0;

	while (42)
	{
		bzero(buffer_recv, 1);
		if (headers_ended == true)
		{
			if (content_type == 1)
			{
				DEBUG("NORMAL TYPE");
				for (int i = 0; i < content_len; i++)
				{
					buffer_recv[0] = 0;
					rc = read(sd, buffer_recv, 1);
					if (buffer_recv[0] == '\r')
						i--;
					if (buffer_recv[0] == '\n' || i + 1 == content_len)
					{
						if (i + 1 == content_len)
							line += buffer_recv[0];
						DEBUG("line size " << line.length());
						request.push_back(line);
						line.clear();
					}
					else
						line += buffer_recv[0];

					if (rc <= 0)
					{
						if (rc == 0)
						{
							DEBUG("Connection closed...");
							return (-1);
						}
						else if (errno != EWOULDBLOCK)
							throw(throwMessageErrno("TO REPLACE BY ERROR PAGE : recv() failed"));
						break;
					}
				}
			}
			else if (content_type == 2)
			{
				int tot_chunk_size = 0;
				while (42)
				{
					int chunk_size = 0;
					while (42)
					{
						buffer_recv[0] = 0;
						rc = read(sd, buffer_recv, 1);
						if (buffer_recv[0] == '\n')
						{
							DEBUG("LINE = " << line);
							if (line == "\r")
							{
								DEBUG("EMPTY");
								line.clear();
								chunk_size = -1;
								break;
							}
							chunk_size = std::stoi(line.c_str(), 0, 16);
							DEBUG("Chunk Size = " << chunk_size);
							line.clear();
							break;
						}
						else 
							line += buffer_recv[0];

						if (rc <= 0)
						{
							if (rc == 0)
							{
								DEBUG("Connection closed...");
								return (-1);
							}
							else if (errno != EWOULDBLOCK)
								throw(throwMessageErrno("TO REPLACE BY ERROR PAGE : recv() failed"));
							return (0);
						}
					}
					
					if (chunk_size == 0)
						break;
					if (chunk_size > 0)
					{
						tot_chunk_size += chunk_size;
						while (chunk_size != 0)
						{
							DEBUG("CHUNCK READ = " << chunk_size);
							char chunk_buffer_recv[chunk_size + 1];
							bzero(chunk_buffer_recv, chunk_size + 1);
							rc = read(sd, chunk_buffer_recv, chunk_size);
							chunk_size -= rc;
							DEBUG("RC = " << rc  << " | first = " << chunk_buffer_recv[0]);
							line = std::string(chunk_buffer_recv);
							request.push_back(line);
							line.clear();
						}
					}
				}
				DEBUG("TOT chunk size = " << tot_chunk_size);
			}
			return (0);
		}
		else
		{
			buffer_recv[0] = 0;
			rc = read(sd, buffer_recv, 1);
		}

		if (buffer_recv[0] == '\n')
		{
			request.push_back(line);
			if (line.length() == 1 && line[0] == '\r')
			{
				DEBUG("END OP");
				headers_ended = true;
			}
			else
			{
				if (line.find("Content-Length: ") != std::string::npos)
				{
					DEBUG("SUB = " << line.substr(std::string("Content-Length: ").length(), line.length() - 2));
					content_len = atoi(line.substr(std::string("Content-Length: ").length(), line.length()).c_str());
					DEBUG("Content-Length IN: " << content_len);
					content_type = 1;
				}
				else if (line.find("Transfer-Encoding: ") != std::string::npos)
				{
					if (line.find("chunked") != std::string::npos)
						content_type = 2;
				}
			}
			line.clear();
		}
		else
			line += buffer_recv[0];

		if (rc <= 0)
		{
			DEBUG("RC = " << rc);
			if (errno == EWOULDBLOCK)
				DEBUG("EWOULDBLOCK = " << rc);
			if (rc == 0)
			{
				DEBUG("Connection closed...");
				return (-1);
			}
			else if (errno != EWOULDBLOCK)
				throw(throwMessageErrno("TO REPLACE BY ERROR PAGE : recv() failed"));
			break;
		}
	}
	return(0);
}

/**
 *  @brief Close the connection with the socket
 *  descriptor (not listener). Remove the socket of
 *  the set to treat.
 * 
 *  @param sd the socket descriptor.
 *  @param max_sd the number of socket.
 *  @param master_set the master set containing all the final
 *      the socket descriptor.
 *  @return the new number of socket descriptor.
 */
int Server::closeConnection(int sd, int max_sd, fd_set *master_set)
{
	close(sd);
	FD_CLR(sd, master_set);
	if (sd == max_sd)
		while (FD_ISSET(max_sd, master_set) == false)
			max_sd -= 1;
	return (max_sd);
}

/**
 *  @brief Give the server name where the client want
 *  to access.
 *  @todo add throw if host not found.
 * 
 *  @param hb the headers block containing the host parameter.
 *  @return the server name.
 */
std::string Server::getServerName(const HeadersBlock& hb)
{
	std::string server_name;

	for (size_t i = 0; i < hb.getHeaderFields().size(); i++)
	{
		DEBUG("Header name = " << hb.getHeaderFields()[i]._field_name)
		if (hb.getHeaderFields()[i]._field_name == "Host")
		{
			size_t pos = hb.getHeaderFields()[i]._field_value.find(':');
			if (pos != std::string::npos)
				server_name = hb.getHeaderFields()[i]._field_value.substr(0, pos);
			else
				server_name = hb.getHeaderFields()[i]._field_value.substr(0, hb.getHeaderFields()[i]._field_value.length());
			DEBUG("Server Name = " << server_name)
			break;
		}
	}
	return (server_name);
}

template <class T>
static bool vector_contain(std::vector<T> tab, T obj)
{
	for (size_t i = 0; i < tab.size(); i++)
		if (tab[i] == obj)
			return (true);
	return (false);
}

void Server::loop()
{
	{
		int actual_check = -1;
		std::vector<int>    checked_ports;
		std::vector<Socket *> actual_set;
		
		for (size_t i = 0; i < this->_sm.getSockets().size(); i++)
		{
			actual_check = this->_sm.getSockets()[i]->getServerConfiguration().port;
			if (vector_contain<int>(checked_ports, actual_check) == false)
			{
				bool has_default = false;
				for (size_t j = 0; j < this->_sm.getSockets().size(); j++)
					if ((int)this->_sm.getSockets()[j]->getServerConfiguration().port == actual_check)
						actual_set.push_back(this->_sm.getSockets()[j]);
				for (size_t f = 0; f < actual_set.size(); f++)
					if (vector_contain<std::string>(actual_set[f]->getServerConfiguration().names, "default_server") == true)
						has_default = true;
				if (has_default == false)
					actual_set[0]->setToDefault();
				checked_ports.push_back(actual_check);
			}
		}
	}
	
	fd_set  working_set;
	fd_set  master_set;
	int     socket_ready;
	int     max_sd;
	SocketManager<SubSocket> sub_sm;

	master_set = this->_sm.getSDSet();
	max_sd = this->_sm.getLastSD();
	while(42)
	{
		working_set = master_set;
		try
		{
			socket_ready = this->waitConnection(&working_set, max_sd);
			for (int i = 0; i <= max_sd && socket_ready > 0; i++)
				if (FD_ISSET(i, &working_set))
				{
					socket_ready--;
					if (this->_sm.hasSD(i))
						max_sd = this->acceptConnection(i, max_sd, &master_set, sub_sm);
					else
					{
						std::vector<std::string> lines;
						if (this->receiveConnection(i, lines) < 0)
							max_sd = this->closeConnection(i, max_sd, &master_set);
						else
						{
							try
							{
								SubSocket client_socket = sub_sm.getBySD(i);
								HeadersBlock test(lines, client_socket.getClientIp());
								std::string server_name = this->getServerName(test);
								Socket last = this->_sm.getBySDandHost(client_socket.getParent().getSocketDescriptor(), server_name);
								size_t len = test.getPlainRequest().length();
								if (test.getPlainRequest() == "\r\n" || len < 9)
									throw(throwMessage("Empty request"));
								DEBUG("TREAT");
								treat(i, working_set, test, last.getServerConfiguration());  //Temporary
								DEBUG("END TREAT");
							}
							catch (const std::exception& e)
							{
								throwError(e);
							}
						}
					}
				}
		}
		catch(const std::exception& e)
		{
			throwError(e);
		}
	}
}