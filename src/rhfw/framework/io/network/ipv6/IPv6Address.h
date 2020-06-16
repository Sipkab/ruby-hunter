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
 * IPv6Address.h
 *
 *  Created on: 2016. aug. 25.
 *      Author: sipka
 */

#ifndef FRAMEWORK_IO_NETWORK_IPV6_IPV6ADDRESS_H_
#define FRAMEWORK_IO_NETWORK_IPV6_IPV6ADDRESS_H_

#include <framework/io/network/NetworkAddress.h>

#include <gen/fwd/types.h>
#include <gen/types.h>
#include <gen/log.h>

namespace rhfw {

class IPv6Address: public NetworkAddress {
protected:
	struct Value {
		uint8 value[16];

		operator uint8*() {
			return value;
		}
		operator const uint8*() const {
			return value;
		}
	} value;

	//TODO
	explicit IPv6Address(NetworkAddressType protocol)
			: NetworkAddress(NetworkAddressType::IPV6 | protocol), value { } {
	}
	IPv6Address(const IPv6Address& o, NetworkAddressType protocol)
			: NetworkAddress((o.getAddressType() & NetworkAddressType::MASK_IP_VERSION) | protocol), value(o.value) {
	}
public:
	IPv6Address()
			: NetworkAddress(NetworkAddressType::IPV6), value { } {
	}

	IPv6Address(const IPv6Address&) = default;
	IPv6Address& operator=(const IPv6Address&) = default;
	const uint8* getAddressBytes() const {
		return value;
	}

	virtual bool operator==(const NetworkAddress& addr) const override {
		//TODO
		return NetworkAddress::operator ==(addr);
	}
};

#if LOGGING_ENABLED
template<>
class __internal_tostring_t<IPv6Address> {
public:
	static _tostring_type tostring(const IPv6Address& value) {
		_tostring_type result = "IP: ";
		for (int i = 0; i < 16; ++i) {
			result = result + TOSTRING((uint32) value.getAddressBytes()[i]) + ".";
		}
		return result;
	}
};
#endif /* RHFW_DEBUG */

}  // namespace rhfw

#endif /* FRAMEWORK_IO_NETWORK_IPV6_IPV6ADDRESS_H_ */
