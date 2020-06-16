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
 * RegistrationToken.h
 *
 *  Created on: 2016. dec. 30.
 *      Author: sipka
 */

#ifndef TEST_SAPPHIRE_SERVER_REGISTRATIONTOKEN_H_
#define TEST_SAPPHIRE_SERVER_REGISTRATIONTOKEN_H_

#include <gen/fwd/types.h>
#include <cstring>

namespace userapp {
using namespace rhfw;

class RegistrationToken {
public:
	uint8 data[256] { };

	operator bool() const {
		uint8 zeros[256] { 0 };
		return memcmp(data, zeros, 256) != 0;
	}

	bool operator !() const {
		uint8 zeros[256] { 0 };
		return memcmp(data, zeros, 256) == 0;
	}
};

} // namespace userapp

#endif /* TEST_SAPPHIRE_SERVER_REGISTRATIONTOKEN_H_ */
