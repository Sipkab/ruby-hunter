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
 * TCPConnection.h
 *
 *  Created on: 2016. aug. 26.
 *      Author: sipka
 */

#ifndef FRAMEWORK_IO_NETWORK_TCP_TCPCONNECTION_H_
#define FRAMEWORK_IO_NETWORK_TCP_TCPCONNECTION_H_

namespace rhfw {
class NetworkAddress;

class TCPConnection {
private:
	virtual int readImpl(void* buffer, unsigned int count) = 0;
	virtual int writeImpl(const void* data, unsigned int count) = 0;

	virtual bool connectImpl(const NetworkAddress& address) = 0;
public:
	TCPConnection();
	TCPConnection(const TCPConnection&) = default;
	TCPConnection(TCPConnection&&) = default;
	TCPConnection& operator=(const TCPConnection&) = default;
	TCPConnection& operator=(TCPConnection&&) = default;
	virtual ~TCPConnection();

	virtual const NetworkAddress& getAddress() const = 0;

	int read(void* buffer, unsigned int count);
	int write(const void* data, unsigned int count);

	bool connect(const NetworkAddress& address);

	virtual void disconnect() = 0;
};

} // namespace rhfw

#endif /* FRAMEWORK_IO_NETWORK_TCP_TCPCONNECTION_H_ */
