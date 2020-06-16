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
 * TCPConnection.cpp
 *
 *  Created on: 2016. aug. 26.
 *      Author: sipka
 */

#include <framework/io/network/tcp/TCPConnection.h>
#include <framework/io/network/NetworkAddress.h>
#include <gen/types.h>

namespace rhfw {

TCPConnection::TCPConnection() {
}
TCPConnection::~TCPConnection() {
}

int TCPConnection::write(const void* data, unsigned int count) {
	return writeImpl(data, count);
}

int TCPConnection::read(void* buffer, unsigned int count) {
	return readImpl(buffer, count);
}

bool TCPConnection::connect(const NetworkAddress& address) {
	ASSERT((address.getAddressType() & (NetworkAddressType::MASK_PROTOCOL)) == NetworkAddressType::PROTOCOL_UDP_TCP);
	return connectImpl(address);
}

} // namespace rhfw
