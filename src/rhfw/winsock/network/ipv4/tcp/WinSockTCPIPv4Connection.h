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
 * Win32TCPIPv4Connection.h
 *
 *  Created on: 2016. aug. 26.
 *      Author: sipka
 */

#ifndef WINSOCK_NETWORK_IPV4_TCP_WINSOCKTCPIPV4CONNECTION_H_
#define WINSOCK_NETWORK_IPV4_TCP_WINSOCKTCPIPV4CONNECTION_H_

#include <framework/io/network/ipv4/tcp/TCPIPv4Connection.h>
#include <framework/io/network/ipv4/tcp/TCPIPv4Address.h>

#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#undef WIN32_LEAN_AND_MEAN

namespace rhfw {

class WinSockTCPIPv4Connection final: public TCPIPv4ConnectionBase {
	friend class WinSockTCPIPv4Socket;
private:
	SOCKET socket = INVALID_SOCKET;

	virtual int readImpl(void* buffer, unsigned int count) override;
	virtual int writeImpl(const void* data, unsigned int count) override;

	virtual bool connectImpl(const NetworkAddress& address) override;

	WinSockTCPIPv4Connection(const TCPIPv4Address& address, SOCKET socket);
public:
	WinSockTCPIPv4Connection();
	~WinSockTCPIPv4Connection();

	virtual void disconnect() override;
};

}  // namespace rhfw

#endif /* WINSOCK_NETWORK_IPV4_TCP_WINSOCKTCPIPV4CONNECTION_H_ */
