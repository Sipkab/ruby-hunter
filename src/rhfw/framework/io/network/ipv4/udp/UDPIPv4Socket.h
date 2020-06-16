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
 * UDPIPv4Socket.h
 *
 *  Created on: 2016. aug. 26.
 *      Author: sipka
 */

#ifndef FRAMEWORK_IO_NETWORK_IPV4_UDP_UDPIPV4SOCKET_H_
#define FRAMEWORK_IO_NETWORK_IPV4_UDP_UDPIPV4SOCKET_H_

#include <framework/io/network/udp/UDPSocket.h>
#include <framework/io/network/ipv4/udp/UDPIPv4Address.h>

#include <gen/platform.h>

namespace rhfw {

class UDPIPv4SocketBase: public UDPSocket {
protected:
	UDPIPv4Address address;
public:
	UDPIPv4SocketBase() {
	}
	explicit UDPIPv4SocketBase(const UDPIPv4Address& address)
			: address(address) {
	}

};

}  // namespace rhfw

#include UDPIPV4SOCKET_EXACT_CLASS_INCLUDE
namespace rhfw {
typedef class UDPIPV4SOCKET_EXACT_CLASS_TYPE UDPIPv4Socket;
}  // namespace rhfw

#endif /* FRAMEWORK_IO_NETWORK_IPV4_UDP_UDPIPV4SOCKET_H_ */
