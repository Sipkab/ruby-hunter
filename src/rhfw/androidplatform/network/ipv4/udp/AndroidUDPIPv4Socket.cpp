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
 * AndroidUDPSocket.cpp
 *
 *  Created on: 2016. aug. 25.
 *      Author: sipka
 */

#include <framework/io/network/ipv4/udp/UDPIPv4Socket.h>
#include <framework/io/byteorder.h>
#include <framework/utils/utility.h>

#include <gen/log.h>

#include <sys/socket.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <errno.h>

namespace rhfw {

AndroidUDPIPv4Socket::AndroidUDPIPv4Socket(const UDPIPv4Address& address)
		: UDPIPv4SocketBase(address) {
}
AndroidUDPIPv4Socket::AndroidUDPIPv4Socket(AndroidUDPIPv4Socket&& o)
		: UDPIPv4SocketBase(util::move(o)), fd(o.fd) {
	o.fd = -1;
}
AndroidUDPIPv4Socket& AndroidUDPIPv4Socket::operator =(AndroidUDPIPv4Socket&& o) {
	UDPSocket::operator =(util::move(o));
	this->fd = o.fd;

	o.fd = -1;
	return *this;
}

AndroidUDPIPv4Socket::~AndroidUDPIPv4Socket() {
	if (fd >= 0) {
		destroy();
	}
}

void AndroidUDPIPv4Socket::initialize() {
	fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
	ASSERT(fd >= 0) << "failed to create socket " << strerror(errno);
	if (fd < 0) {
		return;
	}

	int res;

	int boolarg = 1;
	res = setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &boolarg, sizeof(boolarg));
	ASSERT(res == 0) << "syscall failed " << strerror(errno);

	if (this->broadcast) {
		res = setsockopt(fd, SOL_SOCKET, SO_BROADCAST, &boolarg, sizeof(boolarg));
		ASSERT(res == 0) << "syscall failed " << strerror(errno);
	}

	sockaddr_in saddr;
	saddr.sin_family = AF_INET;
	saddr.sin_port = byteorder::htobe(address.getPort());
	saddr.sin_addr.s_addr = address.getNetworkAddressInt();

	static_assert(INADDR_ANY == 0, "INADDR_ANY is not 0");
	res = bind(fd, (const sockaddr*) &saddr, sizeof(saddr));
	ASSERT(res == 0) << "syscall failed " << strerror(errno) << " address: " << address;
}

void AndroidUDPIPv4Socket::destroy() {
	ASSERT(fd >= 0);

	int res;
	res = close(fd);
	ASSERT(res == 0) << "syscall failed " << strerror(errno);

	fd = -1;
}

bool AndroidUDPIPv4Socket::sendPacketImpl(const NetworkAddress& netaddress, const void* data, unsigned int bytecount) {
	ASSERT((netaddress.getAddressType() & (NetworkAddressType::MASK_IP_VERSION)) == NetworkAddressType::IPV4);
	ASSERT(fd >= 0) << "Socket is not created";

	const UDPIPv4Address& addr = static_cast<const UDPIPv4Address&>(netaddress);

	sockaddr_in saddr;
	saddr.sin_family = AF_INET;
	saddr.sin_port = byteorder::htobe(addr.getPort());
	saddr.sin_addr.s_addr = addr.getNetworkAddressInt();
	int result = sendto(fd, data, bytecount, 0, (const sockaddr*) &saddr, sizeof(saddr));
	LOGI()<< "send result: " << result << " to address: " << addr;
	return true;
}
int AndroidUDPIPv4Socket::receivePacketImpl(void* data, unsigned int bytecount, NetworkAddress* netaddress) {
	ASSERT((netaddress->getAddressType() & (NetworkAddressType::MASK_IP_VERSION)) == NetworkAddressType::IPV4);
	UDPIPv4Address* addr = static_cast<UDPIPv4Address*>(netaddress);

	sockaddr_in saddr;
	socklen_t addrlen = sizeof(saddr);
	int result = ::recvfrom(fd, data, bytecount, 0, (sockaddr*) &saddr, &addrlen);
	if (result >= 0) {
		addr->setPort(byteorder::betoh(saddr.sin_port));
		addr->setNetworkAddressInt(saddr.sin_addr.s_addr);
		return result;
	}
	return -1;
}

} // namespace rhfw

