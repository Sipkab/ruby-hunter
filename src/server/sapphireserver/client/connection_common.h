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
 * connection_common.h
 *
 *  Created on: 2017. jul. 13.
 *      Author: sipka
 */

#ifndef JNI_SAPPHIRESERVER_CLIENT_CONNECTION_COMMON_H_
#define JNI_SAPPHIRESERVER_CLIENT_CONNECTION_COMMON_H_

namespace userapp {
using namespace rhfw;

template<typename T, unsigned int SIZE>
struct LocalArray {
	T data[SIZE];
	operator T *() {
		return data;
	}
	T& operator[](unsigned int index) {
		ASSERT(index < SIZE) << index << " - " << SIZE;
		return data[index];
	}
	const T& operator[](unsigned int index) const {
		ASSERT(index < SIZE) << index << " - " << SIZE;
		return data[index];
	}
};

static bool ValidateName(const FixedString& name) {
	if (name.length() < SAPPHIRE_USERNAME_MIN_LEN) {
		return false;
	}
	for (auto&& c : name) {
		if (c < ' ' || c >= 127) {
			return false;
		}
	}
	return true;
}
static bool ValidateMessage(const FixedString& name) {
	if (name.length() == 0) {
		return false;
	}
	for (auto&& c : name) {
		if (c < ' ' || c >= 127) {
			return false;
		}
	}
	return true;
}

}  // namespace userapp

#define CHECK_CLIENT_ID() if (!clientUUID) { writeError(cmd, SapphireCommError::InvalidUserId); }

#endif /* JNI_SAPPHIRESERVER_CLIENT_CONNECTION_COMMON_H_ */
