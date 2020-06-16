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
 * SapphireUser.h
 *
 *  Created on: 2016. szept. 26.
 *      Author: sipka
 */

#ifndef TEST_SAPPHIRE_COMMUNITY_SAPPHIREUSER_H_
#define TEST_SAPPHIRE_COMMUNITY_SAPPHIREUSER_H_

#include <framework/utils/FixedString.h>
#include <sapphire/level/SapphireUUID.h>
#include <sapphire/sapphireconstants.h>

#include <gen/serialize.h>
#include <gen/types.h>

namespace userapp {
using namespace rhfw;

class SapphireUser {
	FixedString userName;
	SapphireUUID uuid;
public:
	SapphireUser() {
	}

	const FixedString& getUserName() const {
		return userName;
	}
	const SapphireUUID& getUUID() const {
		return uuid;
	}
	FixedString& getUserName() {
		return userName;
	}
	SapphireUUID& getUUID() {
		return uuid;
	}

	bool operator==(const SapphireUser& o) const {
		return userName == o.userName && uuid == o.uuid;
	}
	bool operator!=(const SapphireUser& o) const {
		return userName != o.userName || uuid != o.uuid;
	}
};

} // namespace userapp

namespace rhfw {
using namespace userapp;

template<Endianness ENDIAN>
class SerializeExecutor<SapphireUser, ENDIAN> {
public:
	template<typename InStream>
	static bool deserialize(InStream& is, SapphireUser& outdata) {
		return SerializeHandler<SafeFixedString<SAPPHIRE_LEVEL_TITLE_MAX_LEN>>::deserialize<ENDIAN>(is, outdata.getUserName())
				&& SerializeHandler<userapp::SapphireUUID>::deserialize<ENDIAN>(is, outdata.getUUID());
	}

	template<typename OutStream>
	static bool serialize(OutStream& os, const SapphireUser& data) {
		return SerializeHandler<FixedString>::serialize<ENDIAN>(os, data.getUserName())
				&& SerializeHandler<userapp::SapphireUUID>::serialize<ENDIAN>(os, data.getUUID());
	}
};
}  // namespace rhfw

#endif /* TEST_SAPPHIRE_COMMUNITY_SAPPHIREUSER_H_ */
