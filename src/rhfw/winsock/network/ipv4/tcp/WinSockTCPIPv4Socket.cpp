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
 * WinSockTCPIPv4Socket.cpp
 *
 *  Created on: 2016. aug. 28.
 *      Author: sipka
 */

#include <framework/io/network/ipv4/tcp/TCPIPv4Socket.h>
#include <framework/io/network/ipv4/tcp/TCPIPv4Connection.h>
#include <framework/io/byteorder.h>

namespace rhfw {

WinSockTCPIPv4Socket::WinSockTCPIPv4Socket() {
}
WinSockTCPIPv4Socket::WinSockTCPIPv4Socket(const TCPIPv4Address& address)
		: TCPIPv4SocketBase(address) {
}
WinSockTCPIPv4Socket::~WinSockTCPIPv4Socket() {
	if (socket != INVALID_SOCKET) {
		destroy();
	}
}

void WinSockTCPIPv4Socket::initialize() {
	ASSERT(socket == INVALID_SOCKET) << "Already initialized";

	socket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	ASSERT(socket != INVALID_SOCKET) << "Failed to create socket" << WSAGetLastError();
	if (socket == INVALID_SOCKET) {
		return;
	}
	int res;

	BOOL boolarg = TRUE;
	res = ::setsockopt(socket, SOL_SOCKET, SO_REUSEADDR, (const char*) &boolarg, sizeof(boolarg));
	ASSERT(res == 0) << "WSA call failed " << WSAGetLastError();

	sockaddr_in saddr;
	saddr.sin_family = AF_INET;
	saddr.sin_port = byteorder::htobe(address.getPort());
	saddr.sin_addr.s_addr = address.getNetworkAddressInt();

	static_assert(INADDR_ANY == 0, "INADDR_ANY is not 0");
	res = ::bind(socket, (const sockaddr*) &saddr, sizeof(saddr));
	ASSERT(res == 0) << "WSA call failed " << WSAGetLastError() << " address: " << address;

	res = ::listen(socket, 8);
	ASSERT(res == 0) << "syscall failed " << WSAGetLastError() << " address: " << address;
}

void WinSockTCPIPv4Socket::destroy() {
	ASSERT(socket != INVALID_SOCKET);

	int res;
	res = ::shutdown(socket, SD_BOTH);
	ASSERT(res == 0 || WSAGetLastError() == WSAENOTCONN) << "WSA call failed " << WSAGetLastError();
	res = ::closesocket(socket);
	ASSERT(res == 0) << "WSA call failed " << WSAGetLastError();

	socket = INVALID_SOCKET;
}

TCPIPv4Connection* WinSockTCPIPv4Socket::acceptImpl() {
	sockaddr_in saddr;
	int addrlen = sizeof(saddr);
	SOCKET accepted = ::accept(socket, (sockaddr*) &saddr, &addrlen);
	WARN(accepted == INVALID_SOCKET) << "Failed to accept on socket: " << address << " error: " << WSAGetLastError();
	if (accepted != INVALID_SOCKET) {
		TCPIPv4Address address;
		address.setPort(byteorder::betoh(saddr.sin_port));
		address.setNetworkAddressInt(saddr.sin_addr.s_addr);
		LOGI()<< "TCP accepted connection: " << address;
		return new TCPIPv4Connection(address, accepted);
	}
	return nullptr;
}

}  // namespace rhfw
