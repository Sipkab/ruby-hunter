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
 * AppleTCPIPv4Connection.cpp
 *
 *  Created on: 2016. aug. 26.
 *      Author: sipka
 */

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

AppleTCPIPv4Connection::AppleTCPIPv4Connection() {
}

AppleTCPIPv4Connection::AppleTCPIPv4Connection(const TCPIPv4Address& address, int fd)
		: TCPIPv4ConnectionBase(address), fd(fd) {
}

AppleTCPIPv4Connection::~AppleTCPIPv4Connection() {
	if (fd >= 0) {
		disconnect();
	}
}

bool AppleTCPIPv4Connection::connectImpl(const NetworkAddress& netaddress) {
	ASSERT((netaddress.getAddressType() & (NetworkAddressType::MASK_IP_VERSION)) == NetworkAddressType::IPV4);
	ASSERT(fd < 0) << "TCP socket already connected to: " << address;

	const TCPIPv4Address& address = static_cast<const TCPIPv4Address&>(netaddress);
	LOGI()<< "TCP connecting to: " << address;
	ASSERT(address.getNetworkAddressInt() != 0) << "Invalid network address: " << address;

	fd = ::socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	ASSERT(fd >= 0) << "failed to create socket " << strerror(errno);
	if (fd < 0) {
		return false;
	}
	{
		int value = 1;
		int res = ::setsockopt(fd, SOL_SOCKET, SO_NOSIGPIPE, &value, sizeof(value));
		ASSERT(res == 0) << "syscall failed " << strerror(errno);
	}

	int res;
	sockaddr_in saddr;
	saddr.sin_family = AF_INET;
	saddr.sin_port = byteorder::htobe(address.getPort());
	saddr.sin_addr.s_addr = address.getNetworkAddressInt();
	res = ::connect(fd, (const sockaddr*) &saddr, sizeof(saddr));

	if (res == 0) {
		this->address = address;
		return true;
	}
	LOGW()<< "Failed to connect to address: " << address << " error: " << strerror(errno);
	//do not close the fd, user has to disconnect()
//	res = ::close(fd);
	//ASSERT(res == 0) << "syscall failed " << strerror(errno);
//	fd = -1;
	return false;
}

int AppleTCPIPv4Connection::readImpl(void* buffer, unsigned int count) {
	WARN(fd < 0);
	int res;
	res = ::recv(fd, buffer, count, 0);
	WARN(res < 0) << "recv returned negative: " << strerror(errno);
	return res;
}

int AppleTCPIPv4Connection::writeImpl(const void* data, unsigned int count) {
	WARN(fd < 0);
	int res;
	res = ::send(fd, data, count, 0);
	WARN(res < 0) << "send returned negative: " << strerror(errno);
	return res;
}

void AppleTCPIPv4Connection::disconnect() {
	WARN(fd < 0);
	if (fd < 0) {
		return;
	}

	int res;
	res = ::shutdown(fd, SHUT_RDWR);
	ASSERT(res == 0 || errno == 107 || errno == 57) << "syscall failed " << strerror(errno);
	res = ::close(fd);
	ASSERT(res == 0) << "syscall failed " << strerror(errno);

	fd = -1;

	this->address = TCPIPv4Address { };
}

}
// namespace rhfw

