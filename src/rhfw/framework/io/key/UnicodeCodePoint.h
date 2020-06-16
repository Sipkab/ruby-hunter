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
 * UnicodeCodePoint.h
 *
 *  Created on: 2015 szept. 3
 *      Author: sipka
 */

#ifndef UNICODECODEPOINT_H_
#define UNICODECODEPOINT_H_

#include <gen/fwd/types.h>

namespace rhfw {

class UnicodeCodePoint {
private:
	uint32 value;
public:
	UnicodeCodePoint(uint32 value = 0)
			: value { value } {
	}
	UnicodeCodePoint(int32 value)
			: UnicodeCodePoint { (uint32) value } {
	}
	UnicodeCodePoint(uint16 value)
			: UnicodeCodePoint { (uint32) value } {
	}
	UnicodeCodePoint(int16 value)
			: UnicodeCodePoint { (uint16) value } {
	}
	UnicodeCodePoint(char value)
			: UnicodeCodePoint { (uint8) value } {
	}
	UnicodeCodePoint(unsigned char value)
			: UnicodeCodePoint { (uint32) value } {
	}
	UnicodeCodePoint(signed char value)
			: UnicodeCodePoint { (uint8) value } {
	}

	operator uint32() const {
		return value;
	}
};

}  // namespace rhfw

#endif /* UNICODECODEPOINT_H_ */
