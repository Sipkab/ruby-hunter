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
 * AndroidTCPIPv4Connection.h
 *
 *  Created on: 2016. aug. 26.
 *      Author: sipka
 */

#ifndef ANDROIDPLATFORM_NETWORK_IPV4_TCP_ANDROIDTCPIPV4CONNECTION_H_
#define ANDROIDPLATFORM_NETWORK_IPV4_TCP_ANDROIDTCPIPV4CONNECTION_H_

#include <framework/io/network/ipv4/tcp/TCPIPv4Connection.h>
#include <framework/io/network/ipv4/tcp/TCPIPv4Address.h>

namespace rhfw {

class AndroidTCPIPv4Connection final: public TCPIPv4ConnectionBase {
	friend class AndroidTCPIPv4Socket;
private:
	int fd = -1;

	virtual int readImpl(void* buffer, unsigned int count) override;
	virtual int writeImpl(const void* data, unsigned int count) override;

	virtual bool connectImpl(const NetworkAddress& address) override;

	AndroidTCPIPv4Connection(const TCPIPv4Address& address, int fd);
public:
	AndroidTCPIPv4Connection();
	~AndroidTCPIPv4Connection();

	virtual void disconnect() override;
};

} // namespace rhfw

#endif /* ANDROIDPLATFORM_NETWORK_IPV4_TCP_ANDROIDTCPIPV4CONNECTION_H_ */
