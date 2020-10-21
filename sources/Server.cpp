/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rchallie <rchallie@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/10/21 15:25:08 by rchallie          #+#    #+#             */
/*   Updated: 2020/10/21 18:14:39 by rchallie         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Server.hpp"

//WIP
Server::Server(SocketManager sm)
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
int Server::acceptConnection(int sd, int max_sd, fd_set *master_set)
{
    int new_sd = 0;

    DEBUG("Readable socket")

    do
    {
        if ((new_sd = accept(sd, NULL, NULL)) < 0)
        {
            if (errno != EWOULDBLOCK)
                throw(throwMessageErrno("TO REPLACE BY ERROR PAGE : accept() failed"));
            break;
        }
        
        DEBUG("New connection added")
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
int Server::receiveConnection(int sd, char *buffer, int buffer_size)
{
    int rc = recv(sd, buffer, buffer_size, 0);
    if (rc <= 0)
    {
        if (rc == 0)
            throw(throwMessageErrno("TO REPLACE BY ERROR PAGE : Connection closed\n"));
        else if (errno != EWOULDBLOCK)
            throw(throwMessageErrno("TO REPLACE BY ERROR PAGE : recv() failed\n"));
        return (-1);
    }
    return (0);
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

//WIP
void Server::loop()
{
    fd_set  working_set;
    fd_set  master_set;
    int     socket_ready;
    int     max_sd;

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
                        max_sd = this->acceptConnection(i, max_sd, &master_set);
                    else
                    {
                        char buffer[4000];
                        if (this->receiveConnection(i, buffer, 4000) < 0)
                            break;
                        
                        //Temporary
                        treat(i, buffer);
                        
                        max_sd = this->closeConnection(i, max_sd, &master_set);
                    }
                }
        }
        catch(const std::exception& e)
        {
            throwError(e);
            break;
        }
    }
}