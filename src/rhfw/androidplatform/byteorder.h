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

#ifndef ANDROIDPLATFORM_BYTEORDER_H_
#define ANDROIDPLATFORM_BYTEORDER_H_

//ignore deprecated-register warning coming from swap functions for now, until better alternative
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-register"

#include <endian.h>
#include <gen/fwd/types.h>

namespace rhfw {
namespace byteorder {

static inline int16 htobe(int16 val) {
	return htobe16(val);
}
static inline int32 htobe(int32 val) {
	return htobe32(val);
}
static inline int64 htobe(int64 val) {
	return htobe64(val);
}

static inline int16 htole(int16 val) {
	return htole16(val);
}
static inline int32 htole(int32 val) {
	return htole32(val);
}
static inline int64 htole(int64 val) {
	return htole64(val);
}

static inline int16 letoh(int16 val) {
	return letoh16(val);
}
static inline int32 letoh(int32 val) {
	return letoh32(val);
}
static inline int64 letoh(int64 val) {
	return letoh64(val);
}

static inline int16 betoh(int16 val) {
	return betoh16(val);
}
static inline int32 betoh(int32 val) {
	return betoh32(val);
}
static inline int64 betoh(int64 val) {
	return betoh64(val);
}

static inline uint16 htobe(uint16 val) {
	return (uint16) htobe((int16) val);
}
static inline uint32 htobe(uint32 val) {
	return (uint32) htobe((int32) val);
}
static inline uint64 htobe(uint64 val) {
	return (uint64) htobe((int64) val);
}

static inline uint16 htole(uint16 val) {
	return (uint16) htole((int16) val);
}
static inline uint32 htole(uint32 val) {
	return (uint32) htole((int32) val);
}
static inline uint64 htole(uint64 val) {
	return (uint64) htole((int64) val);
}

static inline uint16 letoh(uint16 val) {
	return (uint16) letoh((int16) val);
}
static inline uint32 letoh(uint32 val) {
	return (uint32) letoh((int32) val);
}
static inline uint64 letoh(uint64 val) {
	return (uint64) letoh((int64) val);
}

static inline uint16 betoh(uint16 val) {
	return (uint16) betoh((int16) val);
}
static inline uint32 betoh(uint32 val) {
	return (uint32) betoh((int32) val);
}
static inline uint64 betoh(uint64 val) {
	return (uint64) betoh((int64) val);
}

}  // namespace byteorder
}  // namespace rhfw

#pragma clang diagnostic pop

#endif /* ANDROIDPLATFORM_BYTEORDER_H_ */
