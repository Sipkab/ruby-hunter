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
 * ConditionalArray.h
 *
 *  Created on: 2016. marc. 30.
 *      Author: sipka
 */

#ifndef FRAMEWORK_UTILS_CONDITIONALARRAY_H_
#define FRAMEWORK_UTILS_CONDITIONALARRAY_H_

#include <framework/utils/utility.h>

namespace rhfw {

template<typename T, unsigned int Size>
class ConditionalArray {
	T data[Size];
public:
	template<typename ... Args>
	ConditionalArray(Args&& ... args)
			: data { util::forward<Args>(args)... } {
	}

	operator T*() {
		return data;
	}

	T* operator ->() {
		return &data;
	}
};

template<typename T>
class ConditionalArray<T, 0> {
public:
	template<typename ... Args>
	ConditionalArray(Args&& ... args) {
	}

	operator T*() {
		return nullptr;
	}

	T* operator ->() {
		return nullptr;
	}
};

}  // namespace rhfw

#endif /* FRAMEWORK_UTILS_CONDITIONALARRAY_H_ */
