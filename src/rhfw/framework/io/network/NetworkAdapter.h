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
 * NetworkAdapter.h
 *
 *  Created on: 2016. aug. 25.
 *      Author: sipka
 */

#ifndef FRAMEWORK_IO_NETWORK_NETWORKADAPTER_H_
#define FRAMEWORK_IO_NETWORK_NETWORKADAPTER_H_

#include <framework/utils/LinkedNode.h>
#include <framework/io/network/ipv4/IPv4Address.h>
#include <framework/utils/FixedString.h>

#include <gen/platform.h>

namespace rhfw {

typedef class NETWORKADAPTER_EXACT_CLASS_TYPE NetworkAdapter;

class NetworkAdapterBase: public LinkedNode<NetworkAdapter> {
protected:
	IPv4Address ipv4Address;
	IPv4Address ipv4BroadcastAddress;
	IPv4Address ipv4SubnetMask;
	FixedString adapterName;

	NetworkAdapterBase() {
	}
public:
	virtual ~NetworkAdapterBase() {
	}

	const IPv4Address& getIpv4Address() const {
		return ipv4Address;
	}

	const IPv4Address& getIpv4BroadcastAddress() const {
		return ipv4BroadcastAddress;
	}

	const IPv4Address& getIpv4SubnetMask() const {
		return ipv4SubnetMask;
	}

	bool isBroadcastSupported() const {
		return ipv4BroadcastAddress.getNetworkAddressInt() != 0;
	}

	const FixedString& getAdapterName() const {
		return adapterName;
	}
};

} // namespace rhfw

#include NETWORKADAPTER_EXACT_CLASS_INCLUDE

#endif /* FRAMEWORK_IO_NETWORK_NETWORKADAPTER_H_ */
