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
 * TrackingResourceBlock.h
 *
 *  Created on: 2016. marc. 30.
 *      Author: sipka
 */

#ifndef FRAMEWORK_RESOURCE_TRACKINGRESOURCEBLOCK_H_
#define FRAMEWORK_RESOURCE_TRACKINGRESOURCEBLOCK_H_

#include <framework/utils/LinkedNode.h>
#include <framework/resource/Resource.h>

namespace rhfw {

template<typename T>
class TrackingResourceBlock: public ResourceBlock, public LinkedNode<T> {
private:
public:
	template<typename ListType>
	TrackingResourceBlock(T* res, ListType& list)
			: ResourceBlock { res } {
		list.addToEnd(*this);
	}
	~TrackingResourceBlock() {
	}
	virtual T* get() override {
		return static_cast<T*>(resource);
	}
};

}  // namespace rhfw

#endif /* FRAMEWORK_UTILS_TRACKINGRESOURCEBLOCK_H_ */
