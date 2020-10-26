/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   SubSocket.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rchallie <rchallie@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/10/24 11:51:19 by rchallie          #+#    #+#             */
/*   Updated: 2020/10/24 19:04:47 by rchallie         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SUBSOCKET_HPP
# define SUBSOCKET_HPP

#include "Socket.hpp"

class SubSocket : public Socket
{
    private:
        Socket  _parent_socket;

    public:
        SubSocket(Socket& parent, int sd);
        SubSocket(const SubSocket& copy);
        ~SubSocket();
        SubSocket &operator=(const Socket& op);

        Socket &getParent();
};

#endif