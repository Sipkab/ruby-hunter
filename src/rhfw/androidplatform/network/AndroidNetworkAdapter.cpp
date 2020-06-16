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
 * AndroidNetworkAdapter.cpp
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

namespace rhfw {

AndroidNetworkAdapter::AndroidNetworkAdapter() {
}

AndroidNetworkAdapter::~AndroidNetworkAdapter() {
}
#define RES_FAILED_CLEANUP(cleanup) if (res != 0) {\
		THROW()<< "syscall failed " << strerror(errno);\
		goto cleanup;\
	}
LinkedList<AndroidNetworkAdapter> AndroidNetworkAdapter::getAdapters() {
	int res;
	LinkedList<AndroidNetworkAdapter> result;
	int fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (fd < 0) {
		THROW()<< "syscall failed " << strerror(errno);
		goto return_cleanup;
	}
	{
		ifconf ifc;
		ifc.ifc_buf = nullptr;
		res = ioctl(fd, SIOCGIFCONF, &ifc);
		RES_FAILED_CLEANUP(socket_cleanup);
		{
			char* array = new char[ifc.ifc_len];
			ifc.ifc_buf = array;
			res = ioctl(fd, SIOCGIFCONF, &ifc);
			RES_FAILED_CLEANUP(array_cleanup);
			{
				auto* ptr = ifc.ifc_req;
				int count = ifc.ifc_len / sizeof(ifreq);
				for (int i = 0; i < count; ++i) {
					const auto& ifr = ptr[i];
					LOGI()<< "Network interface: " << ifr.ifr_name;
					if (ifr.ifr_addr.sa_family != AF_INET) {
						LOGI()<< "Not IPv4 interface";
						continue;
					}
					uint32 ipaddr = reinterpret_cast<const sockaddr_in&>(ifr.ifr_addr).sin_addr.s_addr;
					LOGI()<< "Address: " << IPv4Address(ipaddr);
					if (ioctl(fd, SIOCGIFFLAGS, ptr + i)
							!= 0|| HAS_FLAG(ifr.ifr_flags, IFF_LOOPBACK) || !HAS_FLAG(ifr.ifr_flags, IFF_UP | IFF_RUNNING)) {

						LOGI()<< "Not valid interface: " << ifr.ifr_name;
					} else {
						AndroidNetworkAdapter* adapter = new AndroidNetworkAdapter();
						adapter->adapterName = ifr.ifr_name;
						adapter->ipv4Address = IPv4Address(ipaddr);
						if(HAS_FLAG(ifr.ifr_flags, IFF_BROADCAST) && ioctl(fd, SIOCGIFBRDADDR, ptr + i) == 0) {
							adapter->ipv4BroadcastAddress = IPv4Address(reinterpret_cast<const sockaddr_in&>(ifr.ifr_broadaddr).sin_addr.s_addr);
						}
						if(ioctl(fd, SIOCGIFNETMASK, ptr + i) == 0) {
							adapter->ipv4SubnetMask = IPv4Address(reinterpret_cast<const sockaddr_in&>(ifr.ifr_netmask).sin_addr.s_addr);
						}

						result.addToEnd(*adapter);
					}
				}
			}
			array_cleanup:

			delete[] array;
		}
		socket_cleanup:

		res = close(fd);
		ASSERT(res == 0) << "syscall failed " << strerror(errno);
	}
	return_cleanup:

	return util::move(result);
}

} // namespace rhfw
