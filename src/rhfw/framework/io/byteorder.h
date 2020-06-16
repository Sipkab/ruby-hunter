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
 * byteorder.h
 *
 *  Created on: 2016. jun. 8.
 *      Author: sipka
 */

#ifndef FRAMEWORK_IO_BYTEORDER_H_
#define FRAMEWORK_IO_BYTEORDER_H_

#include <gen/platform.h>
#include PLATFORM_BYTEORDER_HEADER

namespace rhfw {
namespace byteorder {

static inline int8 htobe(int8 val) {
	return val;
}
static inline int8 htole(int8 val) {
	return val;
}
static inline int8 letoh(int8 val) {
	return val;
}
static inline int8 betoh(int8 val) {
	return val;
}

static inline uint8 htobe(uint8 val) {
	return val;
}
static inline uint8 htole(uint8 val) {
	return val;
}
static inline uint8 letoh(uint8 val) {
	return val;
}
static inline uint8 betoh(uint8 val) {
	return val;
}

}  // namespace byteorder
}  // namespace rhfw

#endif /* FRAMEWORK_IO_BYTEORDER_H_ */
