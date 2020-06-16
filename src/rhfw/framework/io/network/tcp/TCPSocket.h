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
 * TCPSocket.h
 *
 *  Created on: 2016. aug. 26.
 *      Author: sipka
 */

#ifndef FRAMEWORK_IO_NETWORK_TCP_TCPSOCKET_H_
#define FRAMEWORK_IO_NETWORK_TCP_TCPSOCKET_H_

#include <framework/io/network/NetworkAddress.h>
#include <framework/io/network/tcp/TCPConnection.h>

namespace rhfw {

class TCPSocket {
protected:
	virtual TCPConnection* acceptImpl() = 0;
public:
	TCPSocket();
	TCPSocket(const TCPSocket&) = default;
	TCPSocket(TCPSocket&&) = default;
	TCPSocket& operator=(const TCPSocket&) = default;
	TCPSocket& operator=(TCPSocket&&) = default;
	virtual ~TCPSocket();

	virtual void initialize() = 0;
	virtual void destroy() = 0;

	TCPConnection* accept();
};

} // namespace rhfw

#endif /* FRAMEWORK_IO_NETWORK_TCP_TCPSOCKET_H_ */
