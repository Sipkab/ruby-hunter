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
 * RC4Cipher.cpp
 *
 *  Created on: 2016. okt. 2.
 *      Author: sipka
 */

#include <util/RC4Cipher.h>
#include <gen/log.h>

namespace userapp {
using namespace rhfw;

RC4Cipher::RC4Cipher(const uint8* key, unsigned int keylen) {
	ASSERT(keylen > 0);

	initCipher(key, keylen);
}
RC4Cipher::~RC4Cipher() {
}

void RC4Cipher::initCipher(const uint8* key, unsigned int keylen) {
	ASSERT(keylen <= 256) << keylen;

	uint8 j;
	unsigned int i;

	for (i = 0; i < 256; i++) {
		perm[i] = i;
	}
	index1 = 0;
	index2 = 0;

	for (j = 0, i = 0; i < 256; i++) {
		j += perm[i] + key[i % keylen];
		uint8 temp = perm[i];
		perm[i] = perm[j];
		perm[j] = temp;
	}
	//drop N
	dropN(768);
}

void RC4Cipher::dropN(unsigned int count) {
	for (unsigned int i = 0; i < count; ++i) {
		index2 += perm[index1];

		uint8 temp = perm[index1];
		perm[index1] = perm[index2];
		perm[index2] = temp;
	}
}

void RC4Cipher::encrypt(const uint8* inbuffer, uint8* outbuf, unsigned int count) {
	for (unsigned int i = 0; i < count; i++) {
		index1++;
		index2 += perm[index1];

		uint8 temp = perm[index1];
		perm[index1] = perm[index2];
		perm[index2] = temp;

		uint8 j = perm[index1] + perm[index2];
		outbuf[i] = inbuffer[i] ^ perm[j];
	}
}

} // namespace userapp
