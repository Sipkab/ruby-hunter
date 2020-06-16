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
 * AppleNetworkAdapter.cpp
 *
 *  Created on: 2016. aug. 25.
 *      Author: sipka
 */

#include <framework/io/network/NetworkAdapter.h>

#include <framework/io/byteorder.h>
#include <framework/utils/utility.h>

#include <gen/log.h>

#include <sys/socket.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <errno.h>
#include <net/if.h>
#include <ifaddrs.h>

namespace rhfw {

AppleNetworkAdapter::AppleNetworkAdapter() {
}
AppleNetworkAdapter::~AppleNetworkAdapter() {
}

LinkedList<AppleNetworkAdapter> AppleNetworkAdapter::getAdapters() {
	int res;
	LinkedList<AppleNetworkAdapter> result;
	int fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (fd < 0) {
		THROW()<< "syscall failed " << strerror(errno);
		goto return_cleanup;
	}
	{
		ifaddrs *ifap;

		if (getifaddrs(&ifap) == 0) {
			for (ifaddrs * it = ifap; it != nullptr; it = it->ifa_next) {
				LOGI()<< "Network interface: " << it->ifa_name;
				if(HAS_FLAG(it->ifa_flags, IFF_LOOPBACK) ) {
					LOGI() << "Loopback interface";
					continue;
				}
				if (!HAS_FLAG(it->ifa_flags, IFF_UP | IFF_RUNNING)) {
					LOGI()<< "Interface not running";
					continue;
				}
				if(it->ifa_addr == nullptr) {
					LOGI() << "No interface address";
					continue;
				}
				if (it->ifa_addr->sa_family != AF_INET) {
					LOGI() << "Interface not AF_INET";
					continue;
				}
				AppleNetworkAdapter* adapter = new AppleNetworkAdapter();
				adapter->adapterName = it->ifa_name;
				adapter->ipv4Address = IPv4Address(reinterpret_cast<const sockaddr_in&>(*it->ifa_addr).sin_addr.s_addr);
				if(it->ifa_netmask != nullptr) {
					adapter->ipv4SubnetMask = IPv4Address(reinterpret_cast<const sockaddr_in&>(*it->ifa_netmask).sin_addr.s_addr);
				}
				if(HAS_FLAG(it->ifa_flags, IFF_BROADCAST) && it->ifa_broadaddr !=nullptr) {
					adapter->ipv4BroadcastAddress = IPv4Address(reinterpret_cast<const sockaddr_in&>(*it->ifa_broadaddr).sin_addr.s_addr);
				}
				LOGI() << "Addresses: " << adapter->ipv4Address << " subnet: " << adapter->ipv4SubnetMask << " broadcast: " << adapter->ipv4BroadcastAddress;
				result.addToEnd(*adapter);
			}
			freeifaddrs(ifap);
		}
	}
	return_cleanup:

	return util::move(result);
}

} // namespace rhfw
