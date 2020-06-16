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
 * ResourcePool.h
 *
 *  Created on: 2016. marc. 7.
 *      Author: sipka
 */

#ifndef RESOURCEPOOL_H_
#define RESOURCEPOOL_H_

#include <framework/resource/Resource.h>

#include <framework/utils/utility.h>

#include <gen/configuration.h>
#include <gen/log.h>

namespace rhfw {

template<typename T>
class ResourcePool {
private:
	unsigned int allocated;
	Resource<T>* arr;

	void grow(unsigned int targetId) {
		if (targetId < allocated)
			return;

		unsigned int oldsize = allocated;
		while (targetId >= allocated) {
			allocated *= 2;
		}
		Resource<T>* narr = new Resource<T> [allocated];
		for (unsigned int i = 0; i < oldsize; ++i) {
			narr[i] = util::move(arr[i]);
		}
		for (unsigned int i = oldsize; i < allocated; ++i) {
			narr[i] = nullptr;
		}

		delete[] arr;
		arr = narr;
	}
public:
	ResourcePool(unsigned int startingSize = 32)
			: allocated { startingSize }, arr { new Resource<T> [allocated] } {
		for (unsigned int i = 0; i < allocated; ++i) {
			arr[i] = nullptr;
		}
	}
	ResourcePool(const ResourcePool&) = delete;
	ResourcePool& operator=(const ResourcePool&) = delete;
	ResourcePool(ResourcePool&& o)
			: allocated { o.allocated }, arr { o.arr } {
		o.arr = nullptr;
	}
	ResourcePool& operator=(ResourcePool&& o) {
		ASSERT(&o != this) << "Self move assignment";
		for (unsigned int i = 0; i < allocated; ++i) {
			delete arr[i];
		}
		delete[] arr;

		this->allocated = o.allocated;
		this->arr = o.arr;
		o.arr = nullptr;
		return *this;
	}

	~ResourcePool() {
		delete[] arr;
	}

	template<typename Factor>
	Resource<T> factory(unsigned int id, Factor f) {
		grow(id);

		ASSERT(arr[id] == nullptr) << "Resource is already created";

		arr[id] = f();
		return arr[id];
	}
	Resource<T> get(unsigned int id) {
		ASSERT(arr[id] != nullptr) << "Resource is not created";
		return arr[id];
	}

	template<typename Factor>
	Resource<T> getOrFactory(unsigned int id, Factor&& f) {
		grow(id);
		if (arr[id] != nullptr) {
			return get(id);
		}
		return factory(id, util::forward<Factor>(f));
	}

	void purge() {
		for (unsigned int i = 0; i < allocated; ++i) {
			arr[i] = nullptr;
		}
	}
};

}  // namespace rhfw

#endif /* RESOURCEPOOL_H_ */
