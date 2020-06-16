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
 * RC4Cipher.h
 *
 *  Created on: 2016. okt. 2.
 *      Author: sipka
 */

#ifndef TEST_UTIL_RC4CIPHER_H_
#define TEST_UTIL_RC4CIPHER_H_

#include <gen/fwd/types.h>

namespace userapp {
using namespace rhfw;

class RC4Cipher {
	uint8 perm[256];
	uint8 index1 = 0;
	uint8 index2 = 0;
public:
	RC4Cipher(const uint8* key, unsigned int keylen);
	RC4Cipher() {
	}
	~RC4Cipher();

	void dropN(unsigned int count);

	void initCipher(const uint8* key, unsigned int keylen);

	void encrypt(const uint8* inbuffer, uint8* outbuf, unsigned int count);
	void encrypt(uint8* buffer, unsigned int count) {
		encrypt(buffer, buffer, count);
	}
	void decrypt(const uint8* inbuffer, uint8* outbuf, unsigned int count) {
		encrypt(inbuffer, outbuf, count);
	}
	void decrypt(uint8* buffer, unsigned int count) {
		decrypt(buffer, buffer, count);
	}
};

} // namespace userapp

#endif /* TEST_UTIL_RC4CIPHER_H_ */
