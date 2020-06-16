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

#ifndef APPLEPLATFORM_BYTEORDER_H_
#define APPLEPLATFORM_BYTEORDER_H_

#include <CoreFoundation/CFByteOrder.h>
#include <gen/fwd/types.h>

namespace rhfw {
namespace byteorder {

static inline uint16 htobe(uint16 val) {
	return CFSwapInt16HostToBig(val);
}
static inline uint32 htobe(uint32 val) {
	return CFSwapInt32HostToBig(val);
}
static inline uint64 htobe(uint64 val) {
	return CFSwapInt64HostToBig(val);
}

static inline uint16 htole(uint16 val) {
	return CFSwapInt16HostToLittle(val);
}
static inline uint32 htole(uint32 val) {
	return CFSwapInt32HostToLittle(val);
}
static inline uint64 htole(uint64 val) {
	return CFSwapInt64HostToLittle(val);
}

static inline uint16 letoh(uint16 val) {
	return CFSwapInt16LittleToHost(val);
}
static inline uint32 letoh(uint32 val) {
	return CFSwapInt32LittleToHost(val);
}
static inline uint64 letoh(uint64 val) {
	return CFSwapInt64LittleToHost(val);
}

static inline uint16 betoh(uint16 val) {
	return CFSwapInt16BigToHost(val);
}
static inline uint32 betoh(uint32 val) {
	return CFSwapInt32BigToHost(val);
}
static inline uint64 betoh(uint64 val) {
	return CFSwapInt64BigToHost(val);
}

static inline int16 htobe(int16 val) {
	return (int16) htobe((uint16) val);
}
static inline int32 htobe(int32 val) {
	return (int32) htobe((uint32) val);
}
static inline int64 htobe(int64 val) {
	return (int64) htobe((uint64) val);
}

static inline int16 htole(int16 val) {
	return (int16) htole((uint16) val);
}
static inline int32 htole(int32 val) {
	return (int32) htole((uint32) val);
}
static inline int64 htole(int64 val) {
	return (int64) htole((uint64) val);
}

static inline int16 letoh(int16 val) {
	return (int16) letoh((uint16) val);
}
static inline int32 letoh(int32 val) {
	return (int32) letoh((uint32) val);
}
static inline int64 letoh(int64 val) {
	return (int64) letoh((uint64) val);
}

static inline int16 betoh(int16 val) {
	return (int16) betoh((uint16) val);
}
static inline int32 betoh(int32 val) {
	return (int32) betoh((uint32) val);
}
static inline int64 betoh(int64 val) {
	return (int64) betoh((uint64) val);
}

}  // namespace byteorder
}  // namespace rhfw

#endif /* APPLEPLATFORM_BYTEORDER_H_ */
