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
 * AndroidTCPIPv4Socket.h
 *
 *  Created on: 2016. aug. 26.
 *      Author: sipka
 */

#ifndef ANDROIDPLATFORM_NETWORK_IPV4_TCP_ANDROIDTCPIPV4SOCKET_H_
#define ANDROIDPLATFORM_NETWORK_IPV4_TCP_ANDROIDTCPIPV4SOCKET_H_

#include <framework/io/network/ipv4/tcp/TCPIPv4Socket.h>
#include <framework/io/network/ipv4/tcp/TCPIPv4Address.h>

namespace rhfw {

class AndroidTCPIPv4Socket final : public TCPIPv4SocketBase {
private:
	int fd = -1;

	virtual TCPIPv4Connection* acceptImpl() override;
public:
	AndroidTCPIPv4Socket();
	AndroidTCPIPv4Socket(const TCPIPv4Address& address);
	~AndroidTCPIPv4Socket();

	virtual void initialize() override;
	virtual void destroy() override;
};

} // namespace rhfw

#endif /* ANDROIDPLATFORM_NETWORK_IPV4_TCP_ANDROIDTCPIPV4SOCKET_H_ */
