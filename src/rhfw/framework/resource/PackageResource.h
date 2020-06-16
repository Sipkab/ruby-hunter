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
 * PackageResource.h
 *
 *  Created on: 2016. marc. 7.
 *      Author: sipka
 */

#ifndef PACKAGERESOURCE_H_
#define PACKAGERESOURCE_H_

#include <framework/resource/ResourcePool.h>
#include <framework/resource/Resource.h>

#include <framework/utils/utility.h>

#include <gen/resources.h>
#include <gen/log.h>

namespace rhfw {

class PackageResource {
private:
	template<typename T, bool Condition>
	class ConditionalObject {
		T data;
	public:
		template<typename ... Args>
		ConditionalObject(Args&& ... args)
				: data { util::forward<Args>(args)... } {
		}

		T* operator ->() {
			return &data;
		}
	};

	template<typename T>
	class ConditionalObject<T, false> {
	public:
		template<typename ... Args>
		ConditionalObject(Args&& ... args) {
		}

		T* operator ->() {
			return nullptr;
		}
	};

	static ConditionalObject<ResourcePool<ShareableResource>, (ResIds::RESOURCE_COUNT > 0)> packageResources;
public:

	template<typename T, typename Factor>
	static Resource<T> getResourceOrFactory(ResId id, Factor f) {
		ASSERT((unsigned int ) id < ResIds::RESOURCE_COUNT) << "Resource id is invalid: " << id;
		return Resource<T> { packageResources->getOrFactory((unsigned int) id, f) };
	}

	static void purge() {
		if (ResIds::RESOURCE_COUNT > 0) {
			packageResources->purge();
		}
	}
};

}  // namespace rhfw

#endif /* PACKAGERESOURCE_H_ */
