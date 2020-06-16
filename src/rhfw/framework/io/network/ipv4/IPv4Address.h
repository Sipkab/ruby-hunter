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
 * IPv4Address.h
 *
 *  Created on: 2016. aug. 25.
 *      Author: sipka
 */

#ifndef FRAMEWORK_IO_NETWORK_IPV4_IPV4ADDRESS_H_
#define FRAMEWORK_IO_NETWORK_IPV4_IPV4ADDRESS_H_

#include <framework/io/network/NetworkAddress.h>

#include <gen/fwd/types.h>
#include <gen/types.h>
#include <gen/log.h>

namespace rhfw {

class IPv4Address: public NetworkAddress {
protected:
	struct Value {
		uint8 value[4];

		operator uint8*() {
			return value;
		}
		operator const uint8*() const {
			return value;
		}
		operator uint32() const {
			return *reinterpret_cast<const uint32*>(value);
		}
		Value& operator=(uint32 val) {
			*reinterpret_cast<uint32*>(value) = val;
			return *this;
		}
		bool operator==(const Value& o) const {
			return (uint32) *this == (uint32) o;
		}
		bool operator!=(const Value& o) const {
			return (uint32) *this != (uint32) o;
		}
	} value;

	IPv4Address(uint8 msb, uint8 b1, uint8 b2, uint8 lsb, NetworkAddressType protocol)
			: NetworkAddress(NetworkAddressType::IPV4 | protocol), value { msb, b1, b2, lsb } {
	}
	IPv4Address(uint32 networkaddressint, NetworkAddressType protocol)
			: NetworkAddress(NetworkAddressType::IPV4 | protocol) {
		setNetworkAddressInt(networkaddressint);
	}
	explicit IPv4Address(NetworkAddressType protocol)
			: NetworkAddress(NetworkAddressType::IPV4 | protocol), value { 0, 0, 0, 0 } {
	}
	IPv4Address(const IPv4Address& o, NetworkAddressType protocol)
			: NetworkAddress((o.getAddressType() & NetworkAddressType::MASK_IP_VERSION) | protocol), value(o.value) {
	}
public:
	IPv4Address(uint8 msb, uint8 b1, uint8 b2, uint8 lsb)
			: NetworkAddress(NetworkAddressType::IPV4), value { msb, b1, b2, lsb } {
	}
	explicit IPv4Address(uint32 networkaddressint)
			: NetworkAddress(NetworkAddressType::IPV4) {
		setNetworkAddressInt(networkaddressint);
	}

	IPv4Address()
			: NetworkAddress(NetworkAddressType::IPV4), value { 0, 0, 0, 0 } {
	}

	IPv4Address(const IPv4Address&) = default;

	IPv4Address& operator=(const IPv4Address&) = default;
	const uint8* getAddressBytes() const {
		return value;
	}
	void setAddressBytes(uint8 msb, uint8 b1, uint8 b2, uint8 lsb) {
		value[0] = msb;
		value[1] = b1;
		value[2] = b2;
		value[3] = lsb;
	}

	uint32 getNetworkAddressInt() const {
		return value;
	}
	void setNetworkAddressInt(uint32 address) {
		value = address;
	}

	virtual bool operator==(const NetworkAddress& addr) const override {
		return NetworkAddress::operator ==(addr) && value == static_cast<const IPv4Address&>(addr).value;
	}
	virtual bool operator!=(const NetworkAddress& addr) const override {
		return NetworkAddress::operator !=(addr) || value != static_cast<const IPv4Address&>(addr).value;
	}
};

#if LOGGING_ENABLED
template<>
class __internal_tostring_t<IPv4Address> {
public:
	static _tostring_type tostring(const IPv4Address& value) {
		_tostring_type result = "IP: ";
		result = result + TOSTRING((uint32) value.getAddressBytes()[0]) + ".";
		result = result + TOSTRING((uint32) value.getAddressBytes()[1]) + ".";
		result = result + TOSTRING((uint32) value.getAddressBytes()[2]) + ".";
		result = result + TOSTRING((uint32) value.getAddressBytes()[3]);
		return result;
	}
};
#endif /* RHFW_DEBUG */

}  // namespace rhfw

#endif /* FRAMEWORK_IO_NETWORK_IPV4_IPV4ADDRESS_H_ */
