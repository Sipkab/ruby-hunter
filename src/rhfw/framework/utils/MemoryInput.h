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
 * MemoryInput.h
 *
 *  Created on: 2016. aug. 24.
 *      Author: sipka
 */

#ifndef FRAMEWORK_XML_MEMORYINPUT_H_
#define FRAMEWORK_XML_MEMORYINPUT_H_

#include <gen/serialize.h>
#include <gen/log.h>

namespace rhfw {

template<typename T>
class MemoryInput {
	T* ptr;
	unsigned int length;
public:
	MemoryInput(T* ptr, unsigned int length)
			: ptr(ptr), length(length) {
	}
	MemoryInput()
			: ptr(nullptr), length(0) {
	}
	MemoryInput(const MemoryInput&) = default;
	MemoryInput(MemoryInput&&) = default;
	MemoryInput& operator=(const MemoryInput&) = default;
	MemoryInput& operator=(MemoryInput&&) = default;
	~MemoryInput() = default;

	T* read(unsigned int count, unsigned int* availableout) {
		auto* result = ptr;
		*availableout = count > length ? length : count;
		ptr += *availableout;
		length -= *availableout;
		return result;
	}
	int read(void* buffer, unsigned int count) {
		if (count > length) {
			count = length;
		}
		memcpy(buffer, ptr, count);
		length -= count;
		reinterpret_cast<const char*&>(ptr) += count;
		return count;
	}

	T* getPointer() const {
		return ptr;
	}
	unsigned int getLength() const {
		return length;
	}

	T& operator[](unsigned int index) const {
		return ptr[index];
	}

	T* begin() const {
		return ptr;
	}
	T* end() const {
		return ptr + length;
	}
};

template<typename T>
class SerializeIsEndianIndependent<MemoryInput<T>> : public SerializeIsEndianIndependent<T> {
};

template<typename T, Endianness ENDIAN>
class SerializeExecutor<MemoryInput<T>, ENDIAN> {
public:
	template<typename InStream>
	static bool deserialize(InStream& is, MemoryInput<T>& outdata) {
		for (auto&& item : outdata) {
			bool success = SerializeExecutor<T, ENDIAN>::template deserialize(item);
			if (!success) {
				return false;
			}
		}
		return true;
	}
	template<typename InStream>
	static bool deserialize(InStream& is, MemoryInput<T> && outdata) {
		return deserialize(is, outdata);
	}

	template<typename OutStream>
	static bool serialize(OutStream& os, const MemoryInput<T>& data) {
		for (auto&& item : data) {
			bool success = SerializeExecutor<T, ENDIAN>::template serialize(item);
			if (!success) {
				return false;
			}
		}
		return true;
	}
};

#if LOGGING_ENABLED
template<typename T>
class __internal_tostring_t<MemoryInput<T>> {
public:
	static _tostring_type tostring(const MemoryInput<T>& value) {
		_tostring_type result = "[";
		for (T* it = value.begin(), *end = value.end(); it != end;) {
			result = result + TOSTRING(*it);
			if (++it != end) {
				result = result + ", ";
			}
		}
		return result + "]";
	}
};
#endif /* RHFW_DEBUG */

}  // namespace rhfw

#endif /* FRAMEWORK_XML_MEMORYINPUT_H_ */
