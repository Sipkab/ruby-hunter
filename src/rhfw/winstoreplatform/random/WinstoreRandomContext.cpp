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
 * WinstoreRandomContext.cpp
 *
 *  Created on: 2016. okt. 7.
 *      Author: sipka
 */

#include <winstoreplatform/random/WinstoreRandomContext.h>
#include <framework/random/PseudoRandomer.h>

#include <Windows.h>
#include <gen/fwd/types.h>

namespace rhfw {

class WinstoreRandomer final: public Randomer {
public:
	WinstoreRandomer() {
	}

	virtual int read(void* buffer, unsigned int count) override {
		Windows::Storage::Streams::IBuffer ^ buffrnd = Windows::Security::Cryptography::CryptographicBuffer::GenerateRandom(count);

		Platform::Array<uint8> ^ platarray;
		Windows::Security::Cryptography::CryptographicBuffer::CopyToByteArray(buffrnd, &platarray);

		auto len = platarray->Length;
		memcpy(buffer, platarray->Data, len);
		return len;
	}
};

WinstoreRandomContext::WinstoreRandomContext() {
}
WinstoreRandomContext::~WinstoreRandomContext() {
}

bool WinstoreRandomContext::load() {
	return true;
}
void WinstoreRandomContext::free() {
}

Randomer* WinstoreRandomContext::createRandomer() {
	return new WinstoreRandomer();
}

} // namespace rhfw

