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
 * SapphireDiscussionMessage.h
 *
 *  Created on: 2016. nov. 19.
 *      Author: sipka
 */

#ifndef TEST_SAPPHIRE_COMMUNITY_SAPPHIREDISCUSSIONMESSAGE_H_
#define TEST_SAPPHIRE_COMMUNITY_SAPPHIREDISCUSSIONMESSAGE_H_

#include <framework/utils/FixedString.h>
#include <sapphire/community/SapphireUser.h>
#include <sapphire/sapphireconstants.h>

#include <gen/types.h>
#include <gen/serialize.h>

namespace userapp {
using namespace rhfw;

class SapphireDiscussionMessage {
public:
	FixedString userName;
	FixedString message;
	SapphireDifficulty difficultyColor;

	const char* getUserName() const {
		return userName;
	}
	SapphireDifficulty getMessageDifficultyColor() const {
		return difficultyColor;
	}
	const char* getMessage() const {
		return message;
	}
};

}  // namespace userapp

namespace rhfw {
using namespace userapp;

template<Endianness ENDIAN>
class SerializeExecutor<SapphireDiscussionMessage, ENDIAN> {
public:
	template<typename InStream>
	static bool deserialize(InStream& is, SapphireDiscussionMessage& outdata) {
		return SerializeHandler<SafeFixedString<SAPPHIRE_USERNAME_MAX_LEN>>::deserialize<ENDIAN>(is, outdata.userName)
				&& SerializeHandler<SapphireDifficulty>::deserialize<ENDIAN>(is, outdata.difficultyColor)
				&& SerializeHandler<SafeFixedString<SAPPHIRE_DISCUSSION_MESSAGE_MAX_LEN>>::deserialize<ENDIAN>(is, outdata.message);
	}

	template<typename OutStream>
	static bool serialize(OutStream& os, const SapphireDiscussionMessage& data) {
		return SerializeHandler<FixedString>::serialize<ENDIAN>(os, data.userName)
				&& SerializeHandler<SapphireDifficulty>::serialize<ENDIAN>(os, data.difficultyColor)
				&& SerializeHandler<FixedString>::serialize<ENDIAN>(os, data.message);
	}
};
}  // namespace rhfw

#endif /* TEST_SAPPHIRE_COMMUNITY_SAPPHIREDISCUSSIONMESSAGE_H_ */
