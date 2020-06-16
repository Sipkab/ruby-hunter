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
 * TCPSocket.cpp
 *
 *  Created on: 2016. aug. 26.
 *      Author: sipka
 */

#include <framework/io/network/tcp/TCPSocket.h>

namespace rhfw {

TCPSocket::TCPSocket() {
}
TCPSocket::~TCPSocket() {
}

TCPConnection* TCPSocket::accept() {
	return acceptImpl();
}

} // namespace rhfw
