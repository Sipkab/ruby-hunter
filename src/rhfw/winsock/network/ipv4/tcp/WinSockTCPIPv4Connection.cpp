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
 * WinSockTCPIPv4Connection.cpp
 *
 *  Created on: 2016. aug. 28.
 *      Author: sipka
 */

#include <framework/io/network/ipv4/tcp/TCPIPv4Connection.h>
#include <framework/io/byteorder.h>

namespace rhfw {

WinSockTCPIPv4Connection::WinSockTCPIPv4Connection() {
}
WinSockTCPIPv4Connection::WinSockTCPIPv4Connection(const TCPIPv4Address& address, SOCKET socket)
		: TCPIPv4ConnectionBase(address), socket(socket) {
}
WinSockTCPIPv4Connection::~WinSockTCPIPv4Connection() {
	if (socket != INVALID_SOCKET) {
		disconnect();
	}
}

int WinSockTCPIPv4Connection::readImpl(void* buffer, unsigned int count) {
	WARN(socket == INVALID_SOCKET);
	int res;
	res = ::recv(socket, (char*) buffer, count, 0);
	WARN(res < 0) << "recv returned negative: " << WSAGetLastError();
	return res;
}

int WinSockTCPIPv4Connection::writeImpl(const void* data, unsigned int count) {
	WARN(socket == INVALID_SOCKET);
	int res;
	res = ::send(socket, (const char*) data, count, 0);
	WARN(res < 0) << "send returned negative: " << WSAGetLastError();
	return res;
}

bool WinSockTCPIPv4Connection::connectImpl(const NetworkAddress& netaddress) {
	ASSERT((netaddress.getAddressType() & (NetworkAddressType::MASK_IP_VERSION)) == NetworkAddressType::IPV4);
	ASSERT(socket == INVALID_SOCKET) << "TCP socket already connected to: " << address;

	const TCPIPv4Address& address = static_cast<const TCPIPv4Address&>(netaddress);
	LOGI()<< "TCP connecting to: " << address;
	ASSERT(address.getNetworkAddressInt() != 0) << "Invalid network address: " << address;

	socket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	ASSERT(socket != INVALID_SOCKET) << "Failed to create socket" << WSAGetLastError();
	if (socket == INVALID_SOCKET) {
		return false;
	}

	int res;
	sockaddr_in saddr;
	saddr.sin_family = AF_INET;
	saddr.sin_port = byteorder::htobe(address.getPort());
	saddr.sin_addr.s_addr = address.getNetworkAddressInt();
	res = ::connect(socket, (const sockaddr*) &saddr, sizeof(saddr));

	if (res == 0) {
		this->address = address;
		return true;
	}
	LOGW()<< "Failed to connect to address: " << address << " error: " << WSAGetLastError();
	//do not close the fd, user has to disconnect()
//	res = ::closesocket(socket);
	//ASSERT(res == 0) << "WSA call failed " << WSAGetLastError();
//	socket = INVALID_SOCKET;
	return false;
}

void WinSockTCPIPv4Connection::disconnect() {
	WARN(socket == INVALID_SOCKET);
	if (socket == INVALID_SOCKET) {
		return;
	}

	int res;
	res = ::shutdown(socket, SD_BOTH);
	ASSERT(res == 0 || WSAGetLastError() == WSAENOTCONN || WSAGetLastError() == WSAECONNRESET) << "WSA call failed " << WSAGetLastError();
	res = ::closesocket(socket);
	ASSERT(res == 0) << "WSA call failed " << WSAGetLastError();

	socket = INVALID_SOCKET;
	this->address = TCPIPv4Address { };
}

}	// namespace rhfw
