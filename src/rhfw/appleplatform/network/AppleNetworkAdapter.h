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
 * AppleNetworkAdapter.h
 *
 *  Created on: 2016. aug. 25.
 *      Author: sipka
 */

#ifndef APPLEPLATFORM_NETWORK_APPLENETWORKADAPTER_H_
#define APPLEPLATFORM_NETWORK_APPLENETWORKADAPTER_H_

#include <framework/io/network/NetworkAdapter.h>
#include <framework/utils/LinkedList.h>

namespace rhfw {

class AppleNetworkAdapter final: public NetworkAdapterBase {
	AppleNetworkAdapter();
public:
	static LinkedList<AppleNetworkAdapter> getAdapters();

	~AppleNetworkAdapter();

	virtual AppleNetworkAdapter* get() override {
		return this;
	}
};

} // namespace rhfw

#endif /* APPLEPLATFORM_NETWORK_APPLENETWORKADAPTER_H_ */
