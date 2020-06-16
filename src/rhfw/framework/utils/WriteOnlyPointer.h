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
 * WriteOnlyPointer.h
 *
 *  Created on: 2016. marc. 25.
 *      Author: sipka
 */

#ifndef WRITEONLYPOINTER_H_
#define WRITEONLYPOINTER_H_

#include <gen/log.h>

namespace rhfw {

template<typename T>
class WriteOnlyPointer;

template<typename T>
class WriteOnlyObject {
private:
	T* ptr;
public:
	WriteOnlyObject(T* ptr)
			: ptr { ptr } {
	}
	WriteOnlyObject<T>& operator=(const T& data) {
		ASSERT(ptr != nullptr) << "Assigning to nullptr";

		*ptr = data;
		return *this;
	}
	WriteOnlyPointer<T> operator &() {
		return ptr;
	}
	explicit operator T&() {
		return ptr;
	}
};

template<typename T>
class WriteOnlyPointer {
private:
	T* ptr;
public:
	WriteOnlyPointer(T* ptr)
			: ptr { ptr } {
	}

	WriteOnlyObject<T> operator*() {
		return ptr;
	}

	WriteOnlyPointer<T>& operator++() {
		++ptr;
		return *this;
	}
	WriteOnlyPointer<T> operator++(int) {
		++ptr;
		return ptr - 1;
	}
	WriteOnlyPointer<T>& operator--() {
		--ptr;
		return *this;
	}
	WriteOnlyPointer<T> operator--(int) {
		--ptr;
		return ptr - 1;
	}
	WriteOnlyPointer<T>& operator+=(int val) {
		ptr += val;
		return *this;
	}
	WriteOnlyPointer<T>& operator+=(unsigned int val) {
		ptr += val;
		return *this;
	}
	WriteOnlyPointer<T>& operator-=(int val) {
		ptr -= val;
		return *this;
	}
	WriteOnlyPointer<T>& operator-=(unsigned int val) {
		ptr -= val;
		return *this;
	}
	WriteOnlyObject<T> operator[](int index) {
		return ptr + index;
	}
	WriteOnlyObject<T> operator[](unsigned int index) {
		return ptr + index;
	}

	WriteOnlyPointer<T> operator+(int val) const {
		return ptr + val;
	}
	WriteOnlyPointer<T> operator+(unsigned int val) const {
		return ptr + val;
	}
	WriteOnlyPointer<T> operator-(int val) const {
		return ptr - val;
	}
	WriteOnlyPointer<T> operator-(unsigned int val) const {
		return ptr - val;
	}

	bool operator ==(const WriteOnlyPointer<T>& o) const {
		return ptr == o.ptr;
	}
	bool operator !=(const WriteOnlyPointer<T>& o) const {
		return ptr != o.ptr;
	}
	bool operator <(const WriteOnlyPointer<T>& o) const {
		return ptr < o.ptr;
	}
	bool operator >(const WriteOnlyPointer<T>& o) const {
		return ptr > o.ptr;
	}
	bool operator <=(const WriteOnlyPointer<T>& o) const {
		return ptr <= o.ptr;
	}
	bool operator >=(const WriteOnlyPointer<T>& o) const {
		return ptr >= o.ptr;
	}
	template<typename CastT>
	WriteOnlyPointer<CastT> reinterpretCast() const {
		return reinterpret_cast<CastT*>(ptr);
	}

	explicit operator T*() {
		return ptr;
	}

};

}  // namespace rhfw

#endif /* WRITEONLYPOINTER_H_ */
