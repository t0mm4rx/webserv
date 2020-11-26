/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rchallie <rchallie@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/10/21 15:25:08 by rchallie          #+#    #+#             */
/*   Updated: 2020/11/26 16:43:34 by rchallie         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Server.hpp"

Server::Server()
:
	_sm(),
	_sub_sm()
{}

Server::Server(SocketManager<Socket *> sm)
:
	_sm(sm),
	_sub_sm()
{}

Server::Server(const Server& copy)
:
	_sm(copy._sm),
	_sub_sm(copy._sub_sm)
{}

Server::~Server()
{}

//WIP
Server &Server::operator=(const Server& op)
{
	if (&op == this)
		return (*this);
	this->_sm = op._sm;
	this->_sub_sm = op._sub_sm;
	return (*this);
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
int Server::acceptConnection(int sd, int max_sd, fd_set *read_set, fd_set *write_set, SocketManager<SubSocket *>& sub_sm)
{
	int new_sd = 0;

	DEBUG("Readable socket")

	sockaddr_in client;
	socklen_t size = sizeof(client);
	if ((new_sd = accept(sd, (sockaddr *)&client, &size)) < 0)
	{
		if (errno != EWOULDBLOCK)
			throw(throwMessageErrno("accept() failed, " + itoa(errno)));
		return (max_sd);
	}

	// std::cout << "new_sd"

	Log("New client connection : " + itoa(new_sd));
	sub_sm.registerSocket(new SubSocket(this->_sm.getBySD(sd), ft_inet_ntoa(client.sin_addr), new_sd));
	FD_SET(new_sd, read_set);
	FD_SET(new_sd, write_set);
	if (new_sd > max_sd)
		max_sd = new_sd;
	return (max_sd);
}

#define BUFFER_SIZE_READ 1000000

/* return 0 = nop, 1 = classic content, 2 = chunked*/
static int hasContent(std::string request)
{
	size_t pos_in = 0;
	std::string line;

	size_t pos = 0;
	size_t end = request.find("\n");
	while (end != std::string::npos)
	{
		line = request.substr(pos, end - pos);
		if ((pos_in = line.find("Transfer-Encoding: chunked")) != std::string::npos
			&& pos_in == 0)
			return (2);
		else if (((pos_in = line.find("Content")) != std::string::npos)
			&& pos_in == 0)
			return (1);
		if ((pos_in = request.find("\r\n\r\n")) != std::string::npos
			&& pos_in == pos)
			break;
		pos = end + 1;
		end = request.find("\n", pos);
	}
	return (0);
}

static size_t getContentLen(std::string request)
{
		size_t pos_in = 0;
	std::string line;

	size_t pos = 0;
	size_t end = request.find("\n");
	while (end != std::string::npos)
	{
		line = request.substr(pos, end - pos);
		if (((pos_in = line.find("Content-Length")) != std::string::npos)
			&& pos_in == 0)
			return (atoi(line.substr(line.find(":") + 2, line.length()).c_str()));
		if ((pos_in = request.find("\r\n\r\n")) != std::string::npos
			&& pos_in == pos)
			break;
		pos = end + 1;
		end = request.find("\n", pos);
	}
	return (0);
}

/**
 *  @brief Set the message get from the socket into
 *  the buffer.
 * 
 *  @return -1 if and error appear, 0 otherwise.
 */
int Server::receiveConnection(int sd, std::string& request)
{
	char buffer_recv[BUFFER_SIZE_READ + 1];
	bzero(buffer_recv, BUFFER_SIZE_READ + 1);
	int rc = 0;
    rc = read(sd, buffer_recv, BUFFER_SIZE_READ);
	if (rc > 0)
	{
		request.append(buffer_recv);
		size_t pos;
		int has_content = hasContent(request);
		if ((pos = request.find("\r\n\r\n")) != std::string::npos && has_content == 0)
			return (0);
		if (has_content > 0)
		{
			std::string rest = request.substr(pos + 4, request.length() - (pos + 4));
			if (has_content != 2 && getContentLen(request) == rest.length())
				return (0);
			
			std::string to_find = "\r\n\r\n";

			if (has_content == 2)
				to_find = "0" + to_find;
			if ((pos = rest.find(to_find)) != std::string::npos)
			{
				if ((has_content == 2 && (pos == 0 || (rest[pos - 1] == '\n' && rest[pos - 2] == '\r')))
					|| has_content == 1)
					return (0);
			}
		}
	}
	else
	{
		Log("Error or connection close on : " + itoa(sd));
		return (-1);
	}
	return (1);
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
int Server::closeConnection(int sd, int max_sd, fd_set *read_set, fd_set *write_set)
{
	Log("Close client connection : " + itoa(sd));
	close(sd);
	FD_CLR(sd, read_set);
	FD_CLR(sd, write_set);
	if (sd == max_sd)
		while (FD_ISSET(max_sd, read_set) == false)
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

/**
 *  @brief Check to be sure that a default
 *  server was selected.
 */
void Server::verifyDefaultServer()
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

void Server::loop()
{
	fd_set  read_set;
	fd_set  write_set;
	
	fd_set  master_rest_set;
	fd_set  master_write_set;
	
	int     max_sd;
	

	master_rest_set = this->_sm.getSDSet();
	FD_ZERO(&master_write_set);
	max_sd = this->_sm.getLastSD();

    verifyDefaultServer();
	while(42)
	{
		read_set = master_rest_set;
		write_set = master_write_set;
		try
		{
			select(max_sd + 1, &read_set, &write_set, NULL, NULL);
			/* Servers */
			for (size_t server = 0; server < this->_sm.getSockets().size(); server++)
			{
				int server_sd = this->_sm.getSockets()[server]->getSocketDescriptor();
				if (FD_ISSET(server_sd, &read_set))
					max_sd = this->acceptConnection(server_sd, max_sd, &master_rest_set, &master_write_set, this->_sub_sm);
			}

			/* Clients */
			for (size_t client = 0; client < this->_sub_sm.getSockets().size(); client++)
			{
				int rtn;
				SubSocket &client_socket = *this->_sub_sm.getSockets()[client];
				int	client_sd = client_socket.getSocketDescriptor();
				bool client_treat = false;
				
				if (FD_ISSET(client_sd, &write_set) && client_socket.informationReceived() == true)
				{
					try
					{
						HeadersBlock header_block(client_socket.getRequest(), client_socket.getClientIp(), hasContent(client_socket.getRequest()));
						Log ("End head request treatment for : " + itoa(client_sd));
						std::string server_name = this->getServerName(header_block);
						Socket *last = this->_sm.getBySDandHost(client_socket.getParent()->getSocketDescriptor(), server_name);
						size_t len = header_block.getPlainRequest().length();
						if (header_block.getPlainRequest() == "\r\n" || len < 9)
							throw(throwMessage("Empty request"));
						if (treat(client_sd, header_block, (*last).getServerConfiguration()) == -1)
						{
							max_sd = this->closeConnection(client_sd, max_sd, &master_rest_set, &master_write_set);
							delete this->_sub_sm.getSockets()[client];
							this->_sub_sm.getSockets().erase(this->_sub_sm.getSockets().begin() + client);
							client--;
							continue;
						}
						client_socket.getRequest().clear();
						client_socket.setReceived(false);
						client_treat = true;
					}
					catch (const std::exception& e)
					{
						throwError(e);
					}
				}

				if (FD_ISSET(client_sd, &read_set) && client_treat == false)
				{
					if ((rtn = this->receiveConnection(client_sd, client_socket.getRequest())) < 0)
					{
						max_sd = this->closeConnection(client_sd, max_sd, &master_rest_set, &master_write_set);
						delete this->_sub_sm.getSockets()[client];
						this->_sub_sm.getSockets().erase(this->_sub_sm.getSockets().begin() + client);
						client--;
					}
					else if (rtn == 0)
						client_socket.setReceived(true);
				}

			}
		} 
		catch(const std::exception& e)
		{
			throwError(e);
		}
	}
}

void Server::closeServer(void)
{
	for (size_t client = 0; client < this->_sub_sm.getSockets().size(); client++)
	{
		close(this->_sub_sm.getSockets()[client]->getSocketDescriptor());
		Log("Client connection closed : " + itoa(this->_sub_sm.getSockets()[client]->getSocketDescriptor()));
		delete this->_sub_sm.getSockets()[client];
	}

	for (size_t server = 0; server < this->_sm.getSockets().size(); server++)
	{
		close(this->_sm.getSockets()[server]->getSocketDescriptor());
		Log("Server closed : " + itoa(this->_sm.getSockets()[server]->getSocketDescriptor()));
		delete this->_sm.getSockets()[server];
	}
}