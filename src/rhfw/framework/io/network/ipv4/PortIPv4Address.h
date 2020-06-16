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
 * PortIPv4Address.h
 *
 *  Created on: 2016. aug. 26.
 *      Author: sipka
 */

#ifndef FRAMEWORK_IO_NETWORK_IPV4_PORTIPV4ADDRESS_H_
#define FRAMEWORK_IO_NETWORK_IPV4_PORTIPV4ADDRESS_H_

#include <framework/io/network/ipv4/IPv4Address.h>

namespace rhfw {

class PortIPv4Address: public IPv4Address {
protected:
	uint16 port;
public:
	PortIPv4Address(const IPv4Address& ipv4, uint16 port = 0)
			: IPv4Address(ipv4, NetworkAddressType::PROTOCOL_UDP_TCP), port(port) {
	}
	explicit PortIPv4Address(uint16 port = 0)
			: IPv4Address(NetworkAddressType::PROTOCOL_UDP_TCP), port(port) {
	}

	uint16 getPort() const {
		return port;
	}
	void setPort(uint16 port) {
		this->port = port;
	}

	virtual bool operator==(const NetworkAddress& addr) const override {
		return IPv4Address::operator ==(addr) && port == static_cast<const PortIPv4Address&>(addr).port;
	}
	virtual bool operator!=(const NetworkAddress& addr) const override {
		return IPv4Address::operator !=(addr) || port != static_cast<const PortIPv4Address&>(addr).port;
	}
};

#if LOGGING_ENABLED
template<>
class __internal_tostring_t<PortIPv4Address> {
public:
	static _tostring_type tostring(const PortIPv4Address& value) {
		_tostring_type result = _tostring_type(TOSTRING(value.getAddressType())) + "(";
		result = result + TOSTRING(static_cast<const IPv4Address&>(value)) + " Port: " + TOSTRING(value.getPort());
		return result + ")";
	}
};
#endif /* RHFW_DEBUG */

}  // namespace rhfw

#endif /* FRAMEWORK_IO_NETWORK_IPV4_PORTIPV4ADDRESS_H_ */
