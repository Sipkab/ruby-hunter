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
 * SapphireLevelDescriptor.h
 *
 *  Created on: 2016. okt. 8.
 *      Author: sipka
 */

#ifndef TEST_SAPPHIRE_SAPPHIRELEVELDESCRIPTOR_H_
#define TEST_SAPPHIRE_SAPPHIRELEVELDESCRIPTOR_H_

#include <framework/utils/FixedString.h>

#include <sapphire/level/Level.h>
#include <sapphire/community/SapphireUser.h>
#include <sapphire/level/LevelAuthor.h>

#include <gen/types.h>

enum class LevelState {
	//in ascending order
	UNSEEN = 0,
	UNFINISHED = 1,
	COMPLETED = 2,

	_count_of_entries = 3,
};

namespace userapp {

class SapphireLevelDescriptor {
private:
	FileDescriptor* fd = nullptr;
public:
	static bool make(SapphireLevelDescriptor* desc, FileDescriptor& fd);
	static int compareUUIDs(const SapphireLevelDescriptor* l, const SapphireLevelDescriptor* r) {
		return l->uuid.compare(r->uuid);
	}
	static int compareAgainstUUID(const SapphireLevelDescriptor* l, const SapphireUUID& uuid) {
		return l->uuid.compare(uuid);
	}
	static SapphireLevelDescriptor* make(FileDescriptor& fd);

	SapphireLevelDescriptor() {
	}
	SapphireLevelDescriptor(const Level& level);
	SapphireLevelDescriptor(SapphireLevelDescriptor&& o);
	~SapphireLevelDescriptor() {
		delete fd;
	}

	SapphireDifficulty difficulty = SapphireDifficulty::Unrated;
	SapphireLevelCategory category = SapphireLevelCategory::None;
	FixedString title;
	unsigned int demoCount = 0;
	LevelState state = LevelState::UNSEEN;
	bool communityLevel = false;
	bool locallyStoredLevel = false;
	LevelAuthor author;

	unsigned int playerCount = 1;

	SapphireUUID uuid;

	bool serverSideAvailable = false;

	int8 userRating = -1;

	bool hasSuspendedGame = false;

	unsigned int levelVersion = 0;

	/**
	 * The time ticks played by the user on this level
	 */
	unsigned int timePlayed = 0;

	SapphireLeaderboards leaderboards = SapphireLeaderboards::NO_FLAG;

	/**
	 * Offset of the latest completed demo in the progress file
	 */
	long long latestProgressFinishStepsOffset = -1;

	bool nonModifyAbleFlag = false;

	const char* levelPack = nullptr;

	bool isEditable() const;
	bool isRateable() const;
	bool isUserRatingReceived() const {
		return userRating >= 0;
	}
	bool isReportable() const {
		return state >= LevelState::UNFINISHED && communityLevel && serverSideAvailable;
	}

	FileDescriptor& getFileDescriptor() const {
		return *fd;
	}
	FileDescriptor* getFileDescriptorPointer() const {
		return fd;
	}
	void setFileDescriptor(FileDescriptor* desc) {
		delete this->fd;
		this->fd = desc;
	}
	FixedString& getTitle() {
		return title;
	}
	const FixedString& getTitle() const {
		return title;
	}

	bool hasLeaderboards() const {
		return leaderboards != SapphireLeaderboards::NO_FLAG;
	}

	bool hasLatestFinishSteps() const {
		return latestProgressFinishStepsOffset >= 0;
	}
};

}  // namespace userapp

#endif /* TEST_SAPPHIRE_SAPPHIRELEVELDESCRIPTOR_H_ */
