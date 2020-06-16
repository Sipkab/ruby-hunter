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
 * UDPSocket.h
 *
 *  Created on: 2016. aug. 25.
 *      Author: sipka
 */

#ifndef FRAMEWORK_IO_NETWORK_IPV4_UDPSOCKET_H_
#define FRAMEWORK_IO_NETWORK_IPV4_UDPSOCKET_H_

#include <framework/io/network/NetworkAdapter.h>
#include <framework/io/network/NetworkAddress.h>

namespace rhfw {

class UDPSocket {
private:
	virtual bool sendPacketImpl(const NetworkAddress& addr, const void* data, unsigned int bytecount) = 0;
	virtual int receivePacketImpl(void* data, unsigned int bytecount, NetworkAddress* addr) = 0;
protected:
	bool broadcast = false;
public:
	UDPSocket() = default;
	UDPSocket(const UDPSocket&) = default;
	UDPSocket(UDPSocket&&) = default;
	UDPSocket& operator=(const UDPSocket&) = default;
	UDPSocket& operator=(UDPSocket&&) = default;
	virtual ~UDPSocket() = default;

	virtual void initialize() = 0;
	virtual void destroy() = 0;

	bool sendPacket(const NetworkAddress& addr, const void* data, unsigned int bytecount);
	int receivePacket(void* data, unsigned int bytecount, NetworkAddress* addr);

	void setBroadcastSocket(bool broadcast) {
		this->broadcast = broadcast;
	}

	bool isBroadcast() const {
		return broadcast;
	}
};

}  // namespace rhfw

#endif /* FRAMEWORK_IO_NETWORK_IPV4_UDPSOCKET_H_ */
