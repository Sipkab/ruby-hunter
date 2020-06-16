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
 * WinSockUDPIPv4Socket.cpp
 *
 *  Created on: 2016. aug. 27.
 *      Author: sipka
 */
#include <framework/io/network/ipv4/udp/UDPIPv4Socket.h>
#include <framework/io/byteorder.h>

#include <gen/log.h>

namespace rhfw {

WinSockUDPIPv4Socket::WinSockUDPIPv4Socket(const UDPIPv4Address& address)
		: UDPIPv4SocketBase(address) {
}
WinSockUDPIPv4Socket::WinSockUDPIPv4Socket(WinSockUDPIPv4Socket&& o)
		: UDPIPv4SocketBase(util::move(o)), socket(o.socket) {
	o.socket = INVALID_SOCKET;
}
WinSockUDPIPv4Socket& WinSockUDPIPv4Socket::operator =(WinSockUDPIPv4Socket&& o) {
	UDPSocket::operator =(util::move(o));
	this->socket = o.socket;

	o.socket = INVALID_SOCKET;
	return *this;
}
WinSockUDPIPv4Socket::~WinSockUDPIPv4Socket() {
	if (socket != INVALID_SOCKET) {
		destroy();
	}
}

bool WinSockUDPIPv4Socket::sendPacketImpl(const NetworkAddress& netaddress, const void* data, unsigned int bytecount) {
	ASSERT((netaddress.getAddressType() & (NetworkAddressType::MASK_IP_VERSION)) == NetworkAddressType::IPV4);
	ASSERT(socket != INVALID_SOCKET) << "Socket is not created";

	const UDPIPv4Address& addr = static_cast<const UDPIPv4Address&>(netaddress);

	sockaddr_in saddr;
	saddr.sin_family = AF_INET;
	saddr.sin_port = byteorder::htobe(addr.getPort());
	saddr.sin_addr.s_addr = addr.getNetworkAddressInt();
	int result = ::sendto(socket, (const char*) data, bytecount, 0, (const sockaddr*) &saddr, sizeof(saddr));
	LOGI()<< "send result: " << result << " to address: " << addr << " error: " << WSAGetLastError();
	return true;
}

int WinSockUDPIPv4Socket::receivePacketImpl(void* data, unsigned int bytecount, NetworkAddress* netaddress) {
	ASSERT((netaddress->getAddressType() & (NetworkAddressType::MASK_IP_VERSION)) == NetworkAddressType::IPV4);
	UDPIPv4Address* addr = static_cast<UDPIPv4Address*>(netaddress);

	sockaddr_in saddr;
	int addrlen = sizeof(saddr);
	int result = ::recvfrom(socket, (char*) data, bytecount, 0, (sockaddr*) &saddr, &addrlen);
	if (result >= 0) {
		addr->setPort(byteorder::betoh(saddr.sin_port));
		addr->setNetworkAddressInt(saddr.sin_addr.s_addr);
		return result;
	}
	return -1;
}

void WinSockUDPIPv4Socket::initialize() {
	socket = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	ASSERT(socket != INVALID_SOCKET) << "Failed to create socket" << WSAGetLastError();
	if (socket == INVALID_SOCKET) {
		return;
	}
	int res;

	BOOL boolarg = TRUE;
	res = ::setsockopt(socket, SOL_SOCKET, SO_REUSEADDR, (const char*) &boolarg, sizeof(boolarg));
	ASSERT(res == 0) << "WSA call failed " << WSAGetLastError();

	if (this->broadcast) {
		res = ::setsockopt(socket, SOL_SOCKET, SO_BROADCAST, (const char*) &boolarg, sizeof(boolarg));
		ASSERT(res == 0) << "WSA call failed " << WSAGetLastError();
	}

	sockaddr_in saddr;
	saddr.sin_family = AF_INET;
	saddr.sin_port = byteorder::htobe(address.getPort());
	saddr.sin_addr.s_addr = address.getNetworkAddressInt();

	static_assert(INADDR_ANY == 0, "INADDR_ANY is not 0");
	res = ::bind(socket, (const sockaddr*) &saddr, sizeof(saddr));
	ASSERT(res == 0) << "WSA call failed " << WSAGetLastError() << " address: " << address;
}

void WinSockUDPIPv4Socket::destroy() {
	ASSERT(socket != INVALID_SOCKET);

	int res = ::closesocket(socket);
	ASSERT(res == 0) << "WSA call failed " << WSAGetLastError();

	socket = INVALID_SOCKET;
}

}  // namespace rhfw
