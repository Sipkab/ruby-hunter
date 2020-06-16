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
 * WinSockNetworkAdapter.cpp
 *
 *  Created on: 2016. aug. 27.
 *      Author: sipka
 */

#include <framework/io/network/NetworkAdapter.h>
#include <framework/io/byteorder.h>

#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <winsock2.h>
#include <iphlpapi.h>
#include <VersionHelpers.h>

#include <gen/log.h>

#pragma comment(lib, "IPHLPAPI.lib")

namespace rhfw {

WinSockNetworkAdapter::WinSockNetworkAdapter() {
}
WinSockNetworkAdapter::~WinSockNetworkAdapter() {
}

LinkedList<WinSockNetworkAdapter> WinSockNetworkAdapter::getAdapters() {
	ULONG buffersize = 1024 * 16;
	char* buffer = new char[buffersize];

	LinkedList<WinSockNetworkAdapter> result;
	ULONG res;
	res = GetAdaptersAddresses(AF_INET, GAA_FLAG_INCLUDE_PREFIX, NULL, (PIP_ADAPTER_ADDRESSES) buffer, &buffersize);
	if (res == ERROR_BUFFER_OVERFLOW) {
		delete[] buffer;
		buffer = new char[buffersize];
		res = GetAdaptersAddresses(AF_INET, GAA_FLAG_INCLUDE_PREFIX, NULL, (PIP_ADAPTER_ADDRESSES) buffer, &buffersize);
	}
	if (res != ERROR_SUCCESS) {
		LOGW()<< "GetAdaptersAddresses returned error: " << res;
		goto nodata_cleanup;
	}
	{
		for (PIP_ADAPTER_ADDRESSES it = reinterpret_cast<PIP_ADAPTER_ADDRESSES>(buffer); it != nullptr; it = it->Next) {
			LOGI()<< "Network adapter found: " << it->AdapterName << " - " << it->FriendlyName << ": " << it->Description;
			if(it->OperStatus != IfOperStatusUp) {
				LOGI() << "Network adapter is not up";
				continue;
			}
			//In case of UWP, IF_TYPE_SOFTWARE_LOOPBACK is not #defined, so we use the numerical value here
			//it is registered with IANA, so not expected to change
			if(it->IfType == 24) {
				LOGI() << "Ignoring loopback adapter";
				continue;
			}
			if(it->FirstUnicastAddress == nullptr) {
				LOGI() << "No first unicast address";
				continue;
			}
			WinSockNetworkAdapter* adapter = new WinSockNetworkAdapter();
			adapter->adapterName = it->FriendlyName;
			adapter->ipv4Address = IPv4Address(reinterpret_cast<sockaddr_in*>(it->FirstUnicastAddress->Address.lpSockaddr)->sin_addr.s_addr);
			if(it->FirstUnicastAddress->OnLinkPrefixLength <= 32) {
				adapter->ipv4SubnetMask = IPv4Address(byteorder::htobe(~(((uint32)0xFFFFFFFF) >> it->FirstUnicastAddress->OnLinkPrefixLength)));
				adapter->ipv4BroadcastAddress = IPv4Address((adapter->ipv4Address.getNetworkAddressInt() & adapter->ipv4SubnetMask.getNetworkAddressInt()) | ~adapter->ipv4SubnetMask.getNetworkAddressInt());
			}
			result.addToEnd(*adapter);
		}
	}

	nodata_cleanup:

	delete[] buffer;
	return util::move(result);
}

}  // namespace rhfw
