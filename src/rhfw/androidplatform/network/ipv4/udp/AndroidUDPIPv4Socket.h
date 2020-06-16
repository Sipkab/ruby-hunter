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
 * AndroidUDPIPv4Socket.h
 *
 *  Created on: 2016. aug. 25.
 *      Author: sipka
 */

#ifndef ANDROIDPLATFORM_NETWORK_ANDROIDUDPIPV4SOCKET_H_
#define ANDROIDPLATFORM_NETWORK_ANDROIDUDPIPV4SOCKET_H_

#include <framework/io/network/ipv4/udp/UDPIPv4Socket.h>
#include <framework/io/network/ipv4/udp/UDPIPv4Address.h>

namespace rhfw {

class AndroidUDPIPv4Socket final: public UDPIPv4SocketBase {
	int fd = -1;

	virtual bool sendPacketImpl(const NetworkAddress& addr, const void* data, unsigned int bytecount) override;
	virtual int receivePacketImpl(void* data, unsigned int bytecount, NetworkAddress* addr) override;
public:

	AndroidUDPIPv4Socket() = default;
	explicit AndroidUDPIPv4Socket(const UDPIPv4Address& address);
	AndroidUDPIPv4Socket(const AndroidUDPIPv4Socket&) = delete;
	AndroidUDPIPv4Socket(AndroidUDPIPv4Socket&& o);
	AndroidUDPIPv4Socket& operator=(const AndroidUDPIPv4Socket&) = delete;
	AndroidUDPIPv4Socket& operator=(AndroidUDPIPv4Socket&& o);
	~AndroidUDPIPv4Socket();

	virtual void initialize() override;
	virtual void destroy() override;
};

} // namespace rhfw

#endif /* ANDROIDPLATFORM_NETWORK_ANDROIDUDPIPV4SOCKET_H_ */
