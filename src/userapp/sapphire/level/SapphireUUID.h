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
 * SapphireUUID.h
 *
 *  Created on: 2016. okt. 3.
 *      Author: sipka
 */

#ifndef TEST_SAPPHIRE_LEVEL_SAPPHIREUUID_H_
#define TEST_SAPPHIRE_LEVEL_SAPPHIREUUID_H_

#include <framework/utils/FixedString.h>

#include <gen/fwd/types.h>
#include <string.h>

namespace userapp {
using namespace rhfw;

class SapphireUUID {
public:
	static const unsigned int UUID_LENGTH = 16;
private:
	unsigned char data[16] { 0 };

	static unsigned int charToValue(unsigned char c) {
		if (c >= '0' && c <= '9') {
			return c - '0';
		}
		if (c >= 'a' && c <= 'f') {
			return c - 'a' + 10;
		}
		return 256;
	}
public:
	SapphireUUID() {
	}
	SapphireUUID(NULLPTR_TYPE) {
	}
	SapphireUUID(unsigned char c00, unsigned char c01, unsigned char c02, unsigned char c03, unsigned char c04, unsigned char c05,
			unsigned char c06, unsigned char c07, unsigned char c08, unsigned char c09, unsigned char c10, unsigned char c11,
			unsigned char c12, unsigned char c13, unsigned char c14, unsigned char c15)
	: data {c00, c01, c02, c03, c04, c05, c06, c07, c08,c09, c10, c11, c12, c13, c14, c15} {
	}
	SapphireUUID(const SapphireUUID&) = default;
	SapphireUUID& operator=(const SapphireUUID&) = default;
	template<typename Char>
	static bool fromString(SapphireUUID* ret, const Char* str, unsigned int strlen) {
		if (strlen < 16 * 2) {
			return false;
		}
		for (unsigned int i = 0; i < 16 * 2; i += 2) {
			unsigned int c1 = charToValue(str[i]);
			unsigned int c2 = charToValue(str[i + 1]);
			if (c1 > 0x0f || c2 > 0x0f) {
				return false;
			}
			ret->data[i / 2] = (((uint8) c1) << 4) | ((uint8) c2);
		}
		return true;
	}
	template<typename Char>
	static bool fromString(SapphireUUID* ret, const Char* str) {
		for (unsigned int i = 0; i < 16 * 2; i += 2) {
			if (str[i] == 0 || str[i + 1] == 0) {
				return false;
			}
			unsigned int c1 = charToValue(str[i]);
			unsigned int c2 = charToValue(str[i + 1]);
			if (c1 > 0x0f || c2 > 0x0f) {
				return false;
			}
			ret->data[i / 2] = (((uint8) c1) << 4) | ((uint8) c2);
		}
		return true;
	}

	static int compareStatic(const SapphireUUID& l, const SapphireUUID& r) {
		return l.compare(r);
	}

	int compare(const SapphireUUID& o) const {
		return memcmp(data, o.data, 16);
	}

	bool operator <(const SapphireUUID& o) const {
		return compare(o) < 0;
	}
	bool operator <=(const SapphireUUID& o) const {
		return compare(o) <= 0;
	}
	bool operator ==(const SapphireUUID& o) const {
		return compare(o) == 0;
	}
	bool operator !=(const SapphireUUID& o) const {
		return compare(o) != 0;
	}
	bool operator >=(const SapphireUUID& o) const {
		return compare(o) >= 0;
	}
	bool operator >(const SapphireUUID& o) const {
		return compare(o) > 0;
	}

	SapphireUUID operator^(const SapphireUUID& o) const {
		SapphireUUID res;
		for (int i = 0; i < UUID_LENGTH; ++i) {
			res.data[i] = this->data[i] ^ o.data[i];
		}
		return res;
	}

	operator bool() const {
		//not all zero
		return compare(SapphireUUID {}) != 0;
	}
	bool operator !() const {
		return !((bool) *this);
	}

	unsigned char* getData() {
		return data;
	}
	const unsigned char* getData() const {
		return data;
	}

	FixedString asString() const {
		uint8 buffer[16 * 2];
		for (unsigned int i = 0; i < 16; ++i) {
			uint8 c = data[i];
			uint8 hi = c >> 4;
			uint8 lo = c & 0x0F;
			buffer[i * 2] = hi >= 10 ? hi + 'a' - 10 : hi + '0';
			buffer[i * 2 + 1] = lo >= 10 ? lo + 'a' - 10 : lo + '0';
		}
		return FixedString {buffer, sizeof(buffer)};
	}

	/**
	 * Data is converted as-is, no byte order is specified
	 */
	uint64 lower64bit() const {
		return *reinterpret_cast<const uint64*>(data + 8);
	}
	/**
	 * Data is converted as-is, no byte order is specified
	 */
	uint64 higher64bit() const {
		return *reinterpret_cast<const uint64*>(data);
	}
};

}
// namespace userapp

#endif /* TEST_SAPPHIRE_LEVEL_SAPPHIREUUID_H_ */
