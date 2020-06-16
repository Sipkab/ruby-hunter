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
 * LinuxUDPIPv4Socket.h
 *
 *  Created on: 2016. aug. 25.
 *      Author: sipka
 */

#ifndef LINUXPLATFORM_NETWORK_LINUXUDPIPV4SOCKET_H_
#define LINUXPLATFORM_NETWORK_LINUXUDPIPV4SOCKET_H_

#include <framework/io/network/ipv4/udp/UDPIPv4Socket.h>
#include <framework/io/network/ipv4/udp/UDPIPv4Address.h>

namespace rhfw {

class LinuxUDPIPv4Socket final: public UDPIPv4SocketBase {
	int fd = -1;

	virtual bool sendPacketImpl(const NetworkAddress& addr, const void* data, unsigned int bytecount) override;
	virtual int receivePacketImpl(void* data, unsigned int bytecount, NetworkAddress* addr) override;
public:

	LinuxUDPIPv4Socket() = default;
	explicit LinuxUDPIPv4Socket(const UDPIPv4Address& address);
	LinuxUDPIPv4Socket(const LinuxUDPIPv4Socket&) = delete;
	LinuxUDPIPv4Socket(LinuxUDPIPv4Socket&& o);
	LinuxUDPIPv4Socket& operator=(const LinuxUDPIPv4Socket&) = delete;
	LinuxUDPIPv4Socket& operator=(LinuxUDPIPv4Socket&& o);
	~LinuxUDPIPv4Socket();

	virtual void initialize() override;
	virtual void destroy() override;
};

} // namespace rhfw

#endif /* LINUXPLATFORM_NETWORK_LINUXUDPIPV4SOCKET_H_ */
