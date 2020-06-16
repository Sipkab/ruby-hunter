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
 * Win32TCPIPv4Socket.h
 *
 *  Created on: 2016. aug. 26.
 *      Author: sipka
 */

#ifndef WINSOCK_NETWORK_IPV4_TCP_WINSOCKTCPIPV4SOCKET_H_
#define WINSOCK_NETWORK_IPV4_TCP_WINSOCKTCPIPV4SOCKET_H_

#include <framework/io/network/ipv4/tcp/TCPIPv4Socket.h>
#include <framework/io/network/ipv4/tcp/TCPIPv4Address.h>
#include <framework/io/network/ipv4/tcp/TCPIPv4Connection.h>

#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#undef WIN32_LEAN_AND_MEAN

namespace rhfw {

class WinSockTCPIPv4Socket final : public TCPIPv4SocketBase {
private:
	SOCKET socket = INVALID_SOCKET;

	virtual TCPIPv4Connection* acceptImpl() override;
public:
	WinSockTCPIPv4Socket();
	explicit WinSockTCPIPv4Socket(const TCPIPv4Address& address);
	~WinSockTCPIPv4Socket();

	virtual void initialize() override;
	virtual void destroy() override;
};

} // namespace rhfw

#endif /* WINSOCK_NETWORK_IPV4_TCP_WINSOCKTCPIPV4SOCKET_H_ */
