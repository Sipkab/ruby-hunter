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
 * LinuxTCPIPv4Socket.cpp
 *
 *  Created on: 2016. aug. 26.
 *      Author: sipka
 */

#include <framework/io/network/ipv4/tcp/TCPIPv4Socket.h>
#include <framework/io/network/ipv4/tcp/TCPIPv4Connection.h>
#include <framework/io/byteorder.h>
#include <gen/log.h>

#include <sys/socket.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <errno.h>

namespace rhfw {

LinuxTCPIPv4Socket::LinuxTCPIPv4Socket() {
}
LinuxTCPIPv4Socket::LinuxTCPIPv4Socket(const TCPIPv4Address& address)
		: TCPIPv4SocketBase(address) {
}
LinuxTCPIPv4Socket::~LinuxTCPIPv4Socket() {
	if (fd >= 0) {
		destroy();
	}
}

void LinuxTCPIPv4Socket::initialize() {
	fd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	ASSERT(fd >= 0) << "failed to create socket " << strerror(errno);
	if (fd < 0) {
		return;
	}

	int res;
	int boolarg = 1;
	res = setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &boolarg, sizeof(boolarg));
	ASSERT(res == 0) << "syscall failed " << strerror(errno);

	sockaddr_in saddr;
	saddr.sin_family = AF_INET;
	saddr.sin_port = byteorder::htobe(address.getPort());
	saddr.sin_addr.s_addr = address.getNetworkAddressInt();

	static_assert(INADDR_ANY == 0, "INADDR_ANY is not 0");
	res = bind(fd, (const sockaddr*) &saddr, sizeof(saddr));
	ASSERT(res == 0) << "syscall failed " << strerror(errno) << " address: " << address;

	res = ::listen(fd, 8);
	ASSERT(res == 0) << "syscall failed " << strerror(errno) << " address: " << address;
}

void LinuxTCPIPv4Socket::destroy() {
	ASSERT(fd >= 0);

	int res;
	res = ::shutdown(fd, SHUT_RDWR);
	ASSERT(res == 0 || errno == 107) << "syscall failed " << strerror(errno);
	res = ::close(fd);
	ASSERT(res == 0) << "syscall failed " << strerror(errno);

	fd = -1;
}

TCPIPv4Connection* LinuxTCPIPv4Socket::acceptImpl() {
	sockaddr_in saddr;
	socklen_t addrlen = sizeof(saddr);
	int accepted = ::accept(fd, (sockaddr*) &saddr, &addrlen);
	WARN(accepted < 0) << "Failed to accept on socket: " << address << " error: " << strerror(errno);
	if (accepted >= 0) {
		TCPIPv4Address address;
		address.setPort(byteorder::betoh(saddr.sin_port));
		address.setNetworkAddressInt(saddr.sin_addr.s_addr);
		LOGI()<< "TCP accepted connection: " << address;
		return new TCPIPv4Connection(address, accepted);
	}
	return nullptr;
}

} // namespace rhfw
