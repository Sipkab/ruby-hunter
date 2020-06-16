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
 * SapphireLevelDescriptor.cpp
 *
 *  Created on: 2016. okt. 8.
 *      Author: sipka
 */

#include <framework/io/files/StorageFileDescriptor.h>
#include <framework/io/stream/BufferedInputStream.h>

#include <sapphire/level/SapphireLevelDescriptor.h>
#include <sapphire/level/Level.h>

#include <sapphire/sapphireconstants.h>

#include <gen/log.h>
#include <gen/serialize.h>
#include <gen/fwd/types.h>

namespace userapp {

SapphireLevelDescriptor::SapphireLevelDescriptor(const Level& level) {
	this->difficulty = level.getInfo().difficulty;
	this->category = level.getInfo().category;
	this->title = level.getInfo().title;
	this->demoCount = level.getDemoCount();
	this->communityLevel = true;
	this->locallyStoredLevel = true;
	this->author = level.getInfo().author;
	this->playerCount = level.getPlayerCount();
	this->uuid = level.getInfo().uuid;
	this->levelVersion = level.getLevelVersion();
}

SapphireLevelDescriptor::SapphireLevelDescriptor(SapphireLevelDescriptor&& o)
		: fd(o.fd), difficulty(o.difficulty), category(o.category), title(util::move(o.title)), demoCount(o.demoCount), state(o.state), communityLevel(
				o.communityLevel), author(util::move(o.author)), playerCount(o.playerCount), uuid(util::move(o.uuid)), serverSideAvailable(
				o.serverSideAvailable), userRating(o.userRating), hasSuspendedGame(o.hasSuspendedGame), levelVersion(o.levelVersion) {
	o.fd = nullptr;
	o.title = nullptr;
	o.playerCount = -1;
	o.uuid = SapphireUUID { };
}

bool SapphireLevelDescriptor::isEditable() const {
	return !serverSideAvailable && locallyStoredLevel && !nonModifyAbleFlag;
}
bool SapphireLevelDescriptor::isRateable() const {
	return serverSideAvailable && communityLevel && locallyStoredLevel && state == LevelState::COMPLETED && isUserRatingReceived();
}

bool SapphireLevelDescriptor::make(SapphireLevelDescriptor* desc, FileDescriptor& fd) {
	auto stream = EndianInputStream<Endianness::Big>::wrap(BufferedInputStream::wrap(fd.openInputStream()));

	uint32 version;
	if (!stream.deserialize<uint32>(version)) {
		return false;
	}

	desc->levelVersion = version;

	if (version > SAPPHIRE_RELEASE_VERSION_NUMBER || version > SAPPHIRE_LEVEL_VERSION_NUMBER) {
		return false;
	}

	while (true) {
		char cmd = 0;
		if (!stream.deserialize<char>(cmd)) {
			return false;
		}
		switch (cmd) {
			case SAPPHIRE_CMD_DEMOCOUNT: { //demo count
				if (!stream.deserialize<uint32>(desc->demoCount)) {
					return false;
				}
				break;
			}
			case 'n': {
				if (!stream.deserialize<SafeFixedString<SAPPHIRE_LEVEL_TITLE_MAX_LEN>>(desc->title)) {
					return false;
				}
				break;
			}
			case 'D': {
				uint32 diff = (uint32) SapphireDifficulty::Unrated;
				if (!stream.deserialize<uint32>(diff) || (unsigned int) diff >= (unsigned int) SapphireDifficulty::_count_of_entries) {
					return false;
				}
				desc->difficulty = (SapphireDifficulty) diff;
				break;
			}
			case 'C': {
				uint32 cat = (uint32) SapphireLevelCategory::None;
				if (!stream.deserialize<uint32>(cat)) {
					return false;
				}
				if (cat < (uint32) SapphireLevelCategory::_count_of_entries) {
					desc->category = (SapphireLevelCategory) cat;
				}
				break;
			}
			case SAPPHIRE_CMD_PLAYERCOUNT: { // player count
				if (!stream.deserialize<uint32>(desc->playerCount)) {
					return false;
				}
				break;
			}
			case SAPPHIRE_CMD_UUID: {
				if (!stream.deserialize<SapphireUUID>(desc->uuid)) {
					return false;
				}
				break;
			}
			case SAPPHIRE_CMD_LEADERBOARDS: {
				if (!stream.deserialize<SapphireLeaderboards>(desc->leaderboards)) {
					return false;
				}
				break;
			}
			case 'a': { // author
				if (!stream.deserialize<SafeFixedString<SAPPHIRE_LEVEL_AUTHOR_MAX_LEN>>(desc->author.getUserName())) {
					return false;
				}
				break;
			}
			case SAPPHIRE_CMD_NON_MODIFYABLE_FLAG: {
				desc->nonModifyAbleFlag = true;
				break;
			}
			default: {
				if (desc->title.length() == 0) {
					return false;
				}
				return true;
			}
		}
	}
	THROW() << "Unreachable";
	return true;
}
SapphireLevelDescriptor* SapphireLevelDescriptor::make(FileDescriptor& fd) {
	SapphireLevelDescriptor* desc = new SapphireLevelDescriptor();
	if (make(desc, fd)) {
		return desc;
	}
	delete desc;
	return nullptr;
}

}  // namespace userapp

