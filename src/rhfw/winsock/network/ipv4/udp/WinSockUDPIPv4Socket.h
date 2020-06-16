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
 * Win32UDPIPv4Socket.h
 *
 *  Created on: 2016. aug. 26.
 *      Author: sipka
 */

#ifndef WINSOCK_NETWORK_IPV4_UDP_WINSOCKUDPIPV4SOCKET_H_
#define WINSOCK_NETWORK_IPV4_UDP_WINSOCKUDPIPV4SOCKET_H_

#include <framework/io/network/ipv4/udp/UDPIPv4Socket.h>
#include <framework/io/network/ipv4/udp/UDPIPv4Address.h>

#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#undef WIN32_LEAN_AND_MEAN

namespace rhfw {

class WinSockUDPIPv4Socket final: public UDPIPv4SocketBase {
	SOCKET socket = INVALID_SOCKET;

	virtual bool sendPacketImpl(const NetworkAddress& addr, const void* data, unsigned int bytecount) override;
	virtual int receivePacketImpl(void* data, unsigned int bytecount, NetworkAddress* addr) override;
public:

	WinSockUDPIPv4Socket() = default;
	explicit WinSockUDPIPv4Socket(const UDPIPv4Address& address);
	WinSockUDPIPv4Socket(const WinSockUDPIPv4Socket&) = delete;
	WinSockUDPIPv4Socket(WinSockUDPIPv4Socket&& o);
	WinSockUDPIPv4Socket& operator=(const WinSockUDPIPv4Socket&) = delete;
	WinSockUDPIPv4Socket& operator=(WinSockUDPIPv4Socket&& o);
	~WinSockUDPIPv4Socket();

	virtual void initialize() override;
	virtual void destroy() override;
};

} // namespace rhfw

#endif /* WINSOCK_NETWORK_IPV4_UDP_WINSOCKUDPIPV4SOCKET_H_ */
