/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rchallie <rchallie@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/10/21 15:25:08 by rchallie          #+#    #+#             */
/*   Updated: 2020/11/23 17:40:14 by rchallie         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Server.hpp"

//WIP
Server::Server(SocketManager<Socket *> sm)
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
int Server::waitConnection(fd_set *read_set, fd_set *write_set, int max_sd)
{
	int socket_ready;

	if ((socket_ready = select(max_sd + 1, read_set,
		write_set, NULL, NULL)) < 0)
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
int Server::acceptConnection(int sd, int max_sd, fd_set *read_set, fd_set *write_set, SocketManager<SubSocket *>& sub_sm)
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
		
		Log("New client connection : " + itoa(new_sd));
		sub_sm.registerSocket(new SubSocket(this->_sm.getBySD(sd), ft_inet_ntoa(client.sin_addr), new_sd));
		FD_SET(new_sd, read_set);
		FD_SET(new_sd, write_set);
		if (new_sd > max_sd)
			max_sd = new_sd;
	} while (new_sd != -1);
	
	return (max_sd);
}

int Server::getChunk(int sd, std::vector<std::string>& request)
{
    char buffer_recv[1];
	int rc = 0;
	std::string line;

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
    return (1);
}

#define BUFFER_SIZE_READ 20

static bool hasContent(std::string request)
{
	size_t pos = 0;
	size_t pos_in = 0;
	std::string line;

	while ((pos = request.find("\n")) != std::string::npos)
	{
		line = request.substr(0, pos);
		if (((pos_in = line.find("Content")) != std::string::npos
			|| (pos_in = line.find("Transfer")) != std::string::npos)
			&& pos_in == 0)
			return (true);
		request.erase(0, pos + 1);
	}
	return (false);
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
		if ((pos = request.find("\r\n\r\n")) != std::string::npos && hasContent(request) == false) // add if body
			return (0);
		else if (hasContent(request) == true)
		{
			std::string rest = request.substr(pos + 4, request.length() - (pos + 4));
			if (rest.find("\r\n\r\n") != std::string::npos)
			{
				std::cout << "Request = [" << request << "]\n";
				return (0);
			}
		}

	}
	else
	{
		std::cout << "RC error = " << rc << std::endl;
		if (errno == EWOULDBLOCK)
			DEBUG("EWOULDBLOCK = " << rc);
		if (rc == 0)
		{
			DEBUG("Connection closed...");
			return (-1);
		}
		else if (errno != EWOULDBLOCK)
			throw(throwMessageErrno("IN TO REPLACE BY ERROR PAGE : recv() failed"));
		else if (request.length() != 0)
		{
			return (0);
		}
	}
	return (1);
}


// int Server::receiveConnection(int sd, std::vector<std::string>& request)
// {
// 	int rc = 0;
// 	char buffer_recv[1];
// 	std::string line;
// 	bool headers_ended = false;
// 	int content_type = 0;
// 	int content_len = 0;

// 	while (42)
// 	{
// 		bzero(buffer_recv, 1);
// 		if (headers_ended == true)
// 		{
// 			if (content_type == 1)
// 			{
// 			    int tot_content_size = 0;
//                 tot_content_size += content_len;
//                 while (content_len != 0)
//                 {
//                     DEBUG("Content_len = " << content_len);
//                     char content_recv[content_len + 1];
//                     bzero(content_recv, content_len + 1);
//                     rc = read(sd, content_recv, content_len);
//                     content_len -= rc;
//                     DEBUG("RC = " << rc  << " | first = " << content_recv[0]);
//                     line = std::string(content_recv);
//                     request.push_back(line);
//                     line.clear();
//                 }
// 				DEBUG("TOT content size = " << tot_content_size);
// 			}
// 			else if (content_type == 2)
// 			{
// 			    int rtn = getChunk(sd, request);
//                 if (rtn != 1)
//                     return (rtn);
// 			}
// 			return (0);
// 		}
// 		else
// 		{
// 			buffer_recv[0] = 0;
// 			rc = read(sd, buffer_recv, 1);
// 		}

// 		if (buffer_recv[0] == '\n')
// 		{
// 			request.push_back(line);
// 			if (line.length() == 1 && line[0] == '\r')
// 			{
// 				DEBUG("END OP");
// 				headers_ended = true;
// 			}
// 			else
// 			{
// 				if (line.find("Content-Length: ") != std::string::npos)
// 				{
// 					DEBUG("SUB = " << line.substr(std::string("Content-Length: ").length(), line.length() - 2));
// 					content_len = atoi(line.substr(std::string("Content-Length: ").length(), line.length()).c_str());
// 					DEBUG("Content-Length IN: " << content_len);
// 					content_type = 1;
// 				}
// 				else if (line.find("Transfer-Encoding: ") != std::string::npos)
// 				{
// 					if (line.find("chunked") != std::string::npos)
// 						content_type = 2;
// 				}
// 			}
// 			line.clear();
// 		}
// 		else
// 			line += buffer_recv[0];

// 		if (rc <= 0)
// 		{
// 			DEBUG("RC = " << rc);
// 			if (errno == EWOULDBLOCK)
// 				DEBUG("EWOULDBLOCK = " << rc);
// 			if (rc == 0)
// 			{
// 				DEBUG("Connection closed...");
// 				return (-1);
// 			}
// 			else if (errno != EWOULDBLOCK)
// 				throw(throwMessageErrno("TO REPLACE BY ERROR PAGE : recv() failed"));
// 			break;
// 		}
// 	}
// 	return(0);
// }

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
	SocketManager<SubSocket *> sub_sm;

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
			this->waitConnection(&read_set, &write_set, max_sd);
			
			/* Servers */
			for (size_t server = 0; server < this->_sm.getSockets().size(); server++)
			{
				int server_sd = this->_sm.getSockets()[server]->getSocketDescriptor();
				if (FD_ISSET(server_sd, &read_set))
				{
					if (this->_sm.getSockets().size() + sub_sm.getSockets().size() < MAX_CONNECTION)
						max_sd = this->acceptConnection(server_sd, max_sd, &master_rest_set, &master_write_set, sub_sm);
					else
						std::cout << "MAX CONNECTION" << std::endl; //rtn 503
				}
			}

			/* Clients */
			for (size_t client = 0; client < sub_sm.getSockets().size(); client++)
			{
				int rtn;
				SubSocket &client_socket = *sub_sm.getSockets()[client];
				int	client_sd = client_socket.getSocketDescriptor();
				
				if (FD_ISSET(client_sd, &write_set) && client_socket.informationReceived() == true)
				{
					try
					{
						HeadersBlock test(client_socket.getRequest(), client_socket.getClientIp());
						std::string server_name = this->getServerName(test);
						Socket *last = this->_sm.getBySDandHost(client_socket.getParent()->getSocketDescriptor(), server_name);
						size_t len = test.getPlainRequest().length();
						if (test.getPlainRequest() == "\r\n" || len < 9)
							throw(throwMessage("Empty request"));
						treat(client_sd, write_set, test, (*last).getServerConfiguration());
						client_socket.getRequest().clear();
						client_socket.setReceived(false);
					}
					catch (const std::exception& e)
					{
						throwError(e);
					}
				}

				if (FD_ISSET(client_sd, &read_set))
				{
					if ((rtn = this->receiveConnection(client_sd, client_socket.getRequest())) < 0)
					{
						max_sd = this->closeConnection(client_sd, max_sd, &master_rest_set, &master_write_set);
						sub_sm.getSockets().erase(sub_sm.getSockets().begin() + client);
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