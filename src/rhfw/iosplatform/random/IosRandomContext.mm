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
 * IosRandomContext.cpp
 *
 *  Created on: 2016. okt. 7.
 *      Author: sipka
 */

#include <iosplatform/random/IosRandomContext.h>
#include <string.h>
#import <Security/SecRandom.h>

namespace rhfw {
class IosRandomer final: public Randomer {
public:
	IosRandomer() {
	}

	virtual int read(void* buffer, unsigned int count) override {
		int res = SecRandomCopyBytes(kSecRandomDefault, count, reinterpret_cast<uint8_t*>(buffer));
		if (res == 0) {
			return count;
		}
		LOGW() << "SecRandomCopyBytes failed: " << strerror(errno);
		return -1;
	}
};

IosRandomContext::IosRandomContext() {
}
IosRandomContext::~IosRandomContext() {
}

bool IosRandomContext::load() {
	return true;
}

void IosRandomContext::free() {
}

Randomer* IosRandomContext::createRandomer() {
	return new IosRandomer();
}

} // namespace rhfw

