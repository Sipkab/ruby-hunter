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
 * NetworkAddress.h
 *
 *  Created on: 2016. aug. 25.
 *      Author: sipka
 */

#ifndef FRAMEWORK_IO_NETWORK_NETWORKADDRESS_H_
#define FRAMEWORK_IO_NETWORK_NETWORKADDRESS_H_

#include <gen/fwd/types.h>
#include <gen/configuration.h>
#include <gen/log.h>

namespace rhfw {

class NetworkAddress {
protected:
	NetworkAddressType addressType;

	NetworkAddress(NetworkAddressType type)
			: addressType(type) {
	}
	NetworkAddress(const NetworkAddress&) = default;
	NetworkAddress& operator=(const NetworkAddress&) = default;
public:
	virtual ~NetworkAddress() = default;

	NetworkAddressType getAddressType() const {
		return addressType;
	}

	virtual bool operator==(const NetworkAddress& addr) const {
		return addressType == addr.addressType;
	}
	virtual bool operator!=(const NetworkAddress& addr) const {
		return !(*this == addr);
	}
};

#if LOGGING_ENABLED
template<>
class __internal_tostring_t<NetworkAddress> {
public:
	static _tostring_type tostring(const NetworkAddress& value) {
		return TOSTRING(value.getAddressType());
	}
};
#endif /* RHFW_DEBUG */

}  // namespace rhfw

#endif /* FRAMEWORK_IO_NETWORK_NETWORKADDRESS_H_ */
