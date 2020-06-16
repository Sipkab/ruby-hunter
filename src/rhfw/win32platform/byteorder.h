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

#ifndef WIN32PLATFORM_BYTEORDER_H_
#define WIN32PLATFORM_BYTEORDER_H_

#include <gen/fwd/types.h>
#include <intrin.h>

/**
 * Here we assume, that Windows operating systems are ALWAYS little endian.
 * Some proof for this: WinSock2.h 10.0.10240.0 version, htonll always swaps bytes
 * Some stackoverflow answer:
 * http://stackoverflow.com/questions/6449468/can-i-safely-assume-that-windows-installations-will-always-be-little-endian
 */

namespace rhfw {
namespace byteorder {

static inline int16 htobe(int16 val) {
	return _byteswap_ushort((unsigned short) val);
}
static inline int32 htobe(int32 val) {
	return _byteswap_ulong((unsigned int) val);
}
static inline int64 htobe(int64 val) {
	return _byteswap_uint64((unsigned __int64) val);
}

static inline int16 htole(int16 val) {
	return val;
}
static inline int32 htole(int32 val) {
	return val;
}
static inline int64 htole(int64 val) {
	return val;
}

static inline int16 letoh(int16 val) {
	return val;
}
static inline int32 letoh(int32 val) {
	return val;
}
static inline int64 letoh(int64 val) {
	return val;
}

static inline int16 betoh(int16 val) {
	return _byteswap_ushort((unsigned short) val);
}
static inline int32 betoh(int32 val) {
	return _byteswap_ulong((unsigned int) val);
}
static inline int64 betoh(int64 val) {
	return _byteswap_uint64((unsigned __int64) val);
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

#endif /* WIN32PLATFORM_BYTEORDER_H_ */
