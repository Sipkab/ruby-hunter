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
 * UDPSocket.cpp
 *
 *  Created on: 2016. aug. 25.
 *      Author: sipka
 */

#include <framework/io/network/udp/UDPSocket.h>
#include <gen/log.h>

namespace rhfw {

bool UDPSocket::sendPacket(const NetworkAddress& address, const void* data, unsigned int bytecount) {
	ASSERT((address.getAddressType() & (NetworkAddressType::MASK_PROTOCOL)) == NetworkAddressType::PROTOCOL_UDP_TCP);
	//for now
	return sendPacketImpl(address, data, bytecount);
}
int UDPSocket::receivePacket(void* data, unsigned int bytecount, NetworkAddress* addr) {
	ASSERT((addr->getAddressType() & (NetworkAddressType::MASK_PROTOCOL)) == NetworkAddressType::PROTOCOL_UDP_TCP);
	return receivePacketImpl(data, bytecount, addr);
}

}  // namespace rhfw

