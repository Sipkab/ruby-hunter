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
 * LinuxNetworkAdapter.h
 *
 *  Created on: 2016. aug. 25.
 *      Author: sipka
 */

#ifndef LINUXPLATFORM_NETWORK_LINUXNETWORKADAPTER_H_
#define LINUXPLATFORM_NETWORK_LINUXNETWORKADAPTER_H_

#include <framework/io/network/NetworkAdapter.h>
#include <framework/utils/LinkedList.h>

namespace rhfw {

class LinuxNetworkAdapter final: public NetworkAdapterBase {
	LinuxNetworkAdapter();
public:
	static LinkedList<LinuxNetworkAdapter> getAdapters();

	~LinuxNetworkAdapter();

	virtual LinuxNetworkAdapter* get() override {
		return this;
	}
};

} // namespace rhfw

#endif /* LINUXPLATFORM_NETWORK_LINUXNETWORKADAPTER_H_ */
