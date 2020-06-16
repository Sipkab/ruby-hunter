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
 * SapphireLevelDetails.h
 *
 *  Created on: 2016. okt. 30.
 *      Author: sipka
 */

#ifndef TEST_SAPPHIRE_SERVER_SAPPHIRELEVELDETAILS_H_
#define TEST_SAPPHIRE_SERVER_SAPPHIRELEVELDETAILS_H_

#include <framework/utils/FixedString.h>

#include <sapphire/level/SapphireLevelDescriptor.h>
#include <sapphire/level/SapphireUUID.h>
#include <sapphire/sapphireconstants.h>

#include <gen/types.h>
#include <gen/serialize.h>

namespace userapp {
using namespace rhfw;

#define RETURN_NONZERO_CMP(expression) do { int cmp = (expression); if(cmp != 0) return cmp; } while(false)

class SapphireLevelDetails {
public:
	static int compareNames(const SapphireLevelDetails& left, const SapphireLevelDetails& right) {
		return left.title.compare(right.title);
	}
	static int compareAuthors(const SapphireLevelDetails& left, const SapphireLevelDetails& right) {
		return left.author.compare(right.author);
	}
	static int compareDates(const SapphireLevelDetails& left, const SapphireLevelDetails& right) {
		RETURN_NONZERO_CMP((int ) left.dateYear - (int ) right.dateYear);
		RETURN_NONZERO_CMP((int ) left.dateMonth - (int ) right.dateMonth);
		RETURN_NONZERO_CMP((int ) left.dateDay - (int ) right.dateDay);
		return 0;
	}
	static int compareRatings(const SapphireLevelDetails& left, const SapphireLevelDetails& right) {
		if (left.ratingCount == 0) {
			if (right.ratingCount == 0) {
				return 0;
			}
			return 1;
		}
		if (right.ratingCount == 0) {
			return -1;
		}
		//descending compare
		//multiply by 1024.0f to differentiate small rating differences
		return (int) ((((float) right.ratingSum / right.ratingCount) - ((float) left.ratingSum / left.ratingCount)) * 1024.0f);
	}

	FixedString title;
	FixedString author;
	SapphireDifficulty difficulty = SapphireDifficulty::Unrated;
	SapphireLevelCategory category = SapphireLevelCategory::None;
	SapphireUUID uuid;

	uint32 playerCount = 0;

	uint32 ratingSum = 0;
	uint32 ratingCount = 0;

	uint32 dateYear = 0;
	uint8 dateMonth = 0;
	uint8 dateDay = 0;

	uint8 userRating = 0;

	SapphireLevelDetails() {
	}

	SapphireLevelDetails& operator=(const SapphireLevelDescriptor& d) {
		this->title = d.title;
		this->author = d.author.getUserName();
		this->difficulty = d.difficulty;
		this->category = d.category;
		this->uuid = d.uuid;
		this->playerCount = d.playerCount;
		return *this;
	}
};

}  // namespace userapp

namespace rhfw {

template<Endianness ENDIAN>
class SerializeExecutor<userapp::SapphireLevelDetails, ENDIAN> {
public:
	template<typename InStream>
	static bool deserialize(InStream& is, userapp::SapphireLevelDetails& outdata) {
		return SerializeHandler<SafeFixedString<SAPPHIRE_LEVEL_TITLE_MAX_LEN>>::deserialize<ENDIAN>(is, outdata.title)
				&& SerializeHandler<SafeFixedString<SAPPHIRE_LEVEL_AUTHOR_MAX_LEN>>::deserialize<ENDIAN>(is, outdata.author)
				&& SerializeHandler<SapphireDifficulty>::deserialize<ENDIAN>(is, outdata.difficulty)
				&& SerializeHandler<SapphireLevelCategory>::deserialize<ENDIAN>(is, outdata.category)
				&& SerializeHandler<userapp::SapphireUUID>::deserialize<ENDIAN>(is, outdata.uuid)
				&& SerializeHandler<uint32>::deserialize<ENDIAN>(is, outdata.playerCount)
				&& SerializeHandler<uint32>::deserialize<ENDIAN>(is, outdata.ratingSum)
				&& SerializeHandler<uint32>::deserialize<ENDIAN>(is, outdata.ratingCount)
				&& SerializeHandler<uint32>::deserialize<ENDIAN>(is, outdata.dateYear)
				&& SerializeHandler<uint8>::deserialize<ENDIAN>(is, outdata.dateMonth)
				&& SerializeHandler<uint8>::deserialize<ENDIAN>(is, outdata.dateDay)
				&& SerializeHandler<uint8>::deserialize<ENDIAN>(is, outdata.userRating);
	}

	template<typename OutStream>
	static bool serialize(OutStream& os, const userapp::SapphireLevelDetails& data) {
		return SerializeHandler<FixedString>::serialize<ENDIAN>(os, data.title)
				&& SerializeHandler<FixedString>::serialize<ENDIAN>(os, data.author)
				&& SerializeHandler<SapphireDifficulty>::serialize<ENDIAN>(os, data.difficulty)
				&& SerializeHandler<SapphireLevelCategory>::serialize<ENDIAN>(os, data.category)
				&& SerializeHandler<userapp::SapphireUUID>::serialize<ENDIAN>(os, data.uuid)
				&& SerializeHandler<uint32>::serialize<ENDIAN>(os, data.playerCount)
				&& SerializeHandler<uint32>::serialize<ENDIAN>(os, data.ratingSum)
				&& SerializeHandler<uint32>::serialize<ENDIAN>(os, data.ratingCount)
				&& SerializeHandler<uint32>::serialize<ENDIAN>(os, data.dateYear)
				&& SerializeHandler<uint8>::serialize<ENDIAN>(os, data.dateMonth)
				&& SerializeHandler<uint8>::serialize<ENDIAN>(os, data.dateDay)
				&& SerializeHandler<uint8>::serialize<ENDIAN>(os, data.userRating);
	}
};

}  // namespace rhfw

#endif /* TEST_SAPPHIRE_SERVER_SAPPHIRELEVELDETAILS_H_ */
