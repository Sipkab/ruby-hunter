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
 * Win32NetworkAdapter.h
 *
 *  Created on: 2016. aug. 26.
 *      Author: sipka
 */

#ifndef WINSOCK_NETWORK_WINSOCKNETWORKADAPTER_H_
#define WINSOCK_NETWORK_WINSOCKNETWORKADAPTER_H_

#include <framework/io/network/NetworkAdapter.h>
#include <framework/utils/LinkedList.h>

namespace rhfw {

class WinSockNetworkAdapter final: public NetworkAdapterBase {
	WinSockNetworkAdapter();
public:
	static LinkedList<WinSockNetworkAdapter> getAdapters();

	~WinSockNetworkAdapter();

	virtual WinSockNetworkAdapter* get() override {
		return this;
	}
};

} // namespace rhfw

#endif /* WINSOCK_NETWORK_WINSOCKNETWORKADAPTER_H_ */
