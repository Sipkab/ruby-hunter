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
 * TCPIPv4Socket.cpp
 *
 *  Created on: 2016. aug. 28.
 *      Author: sipka
 */

#include <framework/io/network/ipv4/tcp/TCPIPv4Socket.h>
#include <gen/log.h>

namespace rhfw {

TCPIPv4SocketBase::TCPIPv4SocketBase(const TCPIPv4Address& address)
		: address(address) {
	ASSERT(address.getPort() != 0);
}

}  // namespace rhfw
