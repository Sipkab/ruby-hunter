/*
 * Copyright (C) 2020 Bence Sipka
 *
 * This program is free software: you can redistribute it and/or modify 
 * it under the terms of the GNU General Public License as published by 
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
/*
 * TCPIPv4Socket.h
 *
 *  Created on: 2016. aug. 26.
 *      Author: sipka
 */

#ifndef FRAMEWORK_IO_NETWORK_IPV4_TCP_TCPIPV4SOCKET_H_
#define FRAMEWORK_IO_NETWORK_IPV4_TCP_TCPIPV4SOCKET_H_

#include <framework/io/network/tcp/TCPSocket.h>
#include <framework/io/network/ipv4/tcp/TCPIPv4Address.h>
#include <framework/io/network/ipv4/tcp/TCPIPv4Connection.h>

#include <gen/platform.h>

namespace rhfw {

class TCPIPv4SocketBase: public TCPSocket {
protected:
	//override the return type covariant
	virtual TCPIPv4Connection* acceptImpl() override = 0;

	TCPIPv4Address address;
public:
	TCPIPv4SocketBase()
			: TCPSocket() {
	}
	explicit TCPIPv4SocketBase(const TCPIPv4Address& address);

	const TCPIPv4Address& getAddress() const {
		return address;
	}
};

}  // namespace rhfw

#include TCPIPV4SOCKET_EXACT_CLASS_INCLUDE
namespace rhfw {
typedef class TCPIPV4SOCKET_EXACT_CLASS_TYPE TCPIPv4Socket;
}  // namespace rhfw

#endif /* FRAMEWORK_IO_NETWORK_IPV4_TCP_TCPIPV4SOCKET_H_ */
