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
 * SapphireDataStorage.h
 *
 *  Created on: 2016. okt. 8.
 *      Author: sipka
 */

#ifndef TEST_SAPPHIRE_SERVER_SAPPHIREDATASTORAGE_H_
#define TEST_SAPPHIRE_SERVER_SAPPHIREDATASTORAGE_H_

#include <framework/utils/BasicListener.h>
#include <framework/utils/ArrayList.h>
#include <sapphire/common/commontypes.h>
#include <gen/fwd/types.h>

namespace userapp {
class Level;
class SapphireUser;
class SapphireUUID;
class RegistrationToken;
class LevelStatistics;
using namespace rhfw;

enum class SapphireStorageError
	: uint32 {

		SUCCESS = 0,
	UNKNOWN_ERROR = 1,
	STORAGE_UNAVAILABLE = 2,
	DEMO_INCORRECT = 3,
	NO_DEMO = 4,
	NO_TITLE = 5,
	LEVEL_SAVE_SUCCESS_DEMO_REMOVED = 6,
	INVALID_USER_UUID = 7,
	NOT_LEVEL_AUTHOR = 8,
	LEVEL_NOT_FOUND = 9,
	LEVEL_ALREADY_EXISTS = 10,
	USER_NOT_FOUND = 11,
	USER_ALREADY_REGISTERED = 12,
	LEVEL_FAILED_TO_LOAD = 13,
	INVALID_RATING = 14,
	NULLPOINTER = 15,
	OUT_OF_BOUNDS = 16,
	INVALID_HARDWARE = 17,
	PROGRESS_UNCHANGED = 18,
	INVALID_PROGRESSID = 19,
	HARDWARE_NOT_ASSOCIATED = 20,
	HARDWARE_ALREADY_ASSOCIATED = 21,
	STATS_NOT_FOUND = 22,
	LEADERBOARD_NOT_FOUND = 23,
	LEVEL_NOT_SUCCESSFULLY_FINISHED = 24,
	DEMO_NOT_FOUND = 25,
};
enum class LevelChangeInfo {
	ADDED = 0,
	RATING_CHANGED = 1,
	REMOVED = 2,
};
enum class SapphireLevelProgress
	:uint32 {
		UNKNOWN = 0xFFFFFFFF,
	LEVEL_SEEN = 0,
	LEVEL_FINISHED = 1,

	MAX_VALUE = 1,
};
SapphireLevelCommProgress sapphireLevelProgressToComm(SapphireLevelProgress progress);
SapphireLevelProgress sapphireLevelCommProgressToSapphireLevelProgress(SapphireLevelCommProgress progress);
inline bool operator !(SapphireStorageError e) {
	return e != SapphireStorageError::SUCCESS;
}

class SapphireLevelDetails;
class SapphireDiscussionMessage;

class SapphireDataStorage {
public:
	class AssociatedHardware {
	public:
		static int compare(const AssociatedHardware* l, const AssociatedHardware* r) {
			return l->hardwareUUID.compare(r->hardwareUUID);
		}
		static int compareUUID(const AssociatedHardware* l, const SapphireUUID& uuid) {
			return l->hardwareUUID.compare(uuid);
		}

		SapphireUUID hardwareUUID;
		ProgressSynchId synchronizedProgressCount = 0;

		AssociatedHardware() {
		}
		AssociatedHardware(const SapphireUUID& uuid, ProgressSynchId synch)
				: hardwareUUID(uuid), synchronizedProgressCount(synch) {
		}
	};
	class LeaderboardEntry {
	public:
		uint32 score;
		FixedString userName;
		PlayerDemoId demoId;
	};
	/**
	 * Param: Index of the level changed
	 */
	using LevelChangedListener = SimpleListener<void(unsigned int, LevelChangeInfo)>;
	/**
	 * Params:
	 * unsigned int: start index of messages queriable
	 * unsigned int: count of all messages
	 */
	using MessagesChangedListener = SimpleListener<void(unsigned int, unsigned int)>;

	/**
	 * Params:
	 * ProgressSynchId: synch id
	 * SapphireUUID: level uuid
	 * SapphireLevelProgress: the progress
	 */
	using HardwareProgressChangedListener = SimpleListener<void(ProgressSynchId, const SapphireUUID&, SapphireLevelProgress)>;

	/**
	 * const AssociatedHardware&: first hardware
	 * const AssociatedHardware&: second hardware
	 * bool: true if association is created, false if removed
	 */
	using HardwareAssociationListener = SimpleListener<void(const AssociatedHardware&, const AssociatedHardware&, bool)>;

	SapphireDataStorage() {
	}
	virtual ~SapphireDataStorage() {
	}

	virtual SapphireStorageError queryLevels(SapphireLevelDetails* details, unsigned int maxcount, unsigned int start,
			unsigned int *outcount, const SapphireUUID& user) = 0;
	virtual SapphireStorageError queryMessages(SapphireDiscussionMessage* messages, unsigned int maxcount, unsigned int* inoutstart,
			unsigned int* outcount) = 0;
	virtual SapphireStorageError queryAssociatedHardwares(const SapphireUUID& hardwareuuid, ArrayList<AssociatedHardware>& outids) = 0;
	virtual SapphireStorageError queryLevelProgress(ProgressSynchId* progressid, const SapphireUUID& hardware, SapphireUUID* outlevel,
			SapphireLevelProgress* outprogress) = 0;

	virtual SapphireStorageError createHardwareAssociation(const SapphireUUID& hardware1, const SapphireUUID& hardware2) = 0;

	virtual SapphireStorageError getLevel(const SapphireUUID& uuid, Level* outlevel, bool* outisbuiltin) = 0;

	virtual SapphireStorageError saveLevel(const Level& level, const SapphireUUID& author) = 0;
	virtual SapphireStorageError removeLevel(const SapphireUUID& leveluuid) = 0;

	virtual SapphireStorageError loginUser(const SapphireUUID& uuid, RegistrationToken* outtoken, FixedString* outname,
			SapphireDifficulty* outdiffcolor) = 0;
	virtual SapphireStorageError registerUser(const SapphireUUID& uuid, const RegistrationToken& token) = 0;
	virtual SapphireStorageError updateUserInfo(const SapphireUUID& uuid, const FixedString& name, SapphireDifficulty diffcolor) = 0;

	virtual SapphireStorageError rateLevel(const SapphireUUID& userid, const SapphireUUID& leveluuid, unsigned int rating) = 0;

	virtual SapphireStorageError appendMessage(const SapphireUUID& userid, const char* message) = 0;

	virtual SapphireStorageError setLevelProgress(ProgressSynchId* progressid, const SapphireUUID& hardware, const SapphireUUID& level,
			SapphireLevelProgress progress) = 0;
	virtual SapphireStorageError getProgressId(const SapphireUUID& hardware, ProgressSynchId* outprogressid) = 0;

	virtual SapphireStorageError getAssociatedHardwareProgressId(const SapphireUUID& hardware, const SapphireUUID& associatedhardware,
			ProgressSynchId* outprogressid) = 0;
	virtual SapphireStorageError increaseAssociatedHardwareProgressId(const SapphireUUID& hardware, const SapphireUUID& associatedhardware,
			ProgressSynchId* progressid) = 0;

	virtual SapphireStorageError addLevelChangedListener(LevelChangedListener& listener) = 0;
	virtual SapphireStorageError removeLevelChangedListener(LevelChangedListener& listener) = 0;

	virtual SapphireStorageError addMessagesChangedListener(MessagesChangedListener& listener) = 0;
	virtual SapphireStorageError removeMessagesChangedListener(MessagesChangedListener& listener) = 0;

	virtual SapphireStorageError addHardwareProgressChangedListener(const SapphireUUID& hardware,
			HardwareProgressChangedListener& listener) = 0;
	virtual SapphireStorageError removeHardwareProgressChangedListener(const SapphireUUID& hardware,
			HardwareProgressChangedListener& listener) = 0;

	virtual SapphireStorageError addHardwareAssociationListener(HardwareAssociationListener& listener) = 0;
	virtual SapphireStorageError removeHardwareAssociationListener(HardwareAssociationListener& listener) = 0;

	virtual SapphireStorageError getLevelStatistics(const SapphireUUID& leveluuid, LevelStatistics* outstats,
			unsigned int* outplaycount) = 0;
	virtual SapphireStorageError appendLevelStatistics(const SapphireUUID& leveluuid, const SapphireUUID& userid, const FixedString& steps,
			uint32 randomseed) = 0;

	virtual SapphireStorageError getPlayerDemo(const SapphireUUID& leveluuid, PlayerDemoId demoid, FixedString* outsteps,
			uint32* outrandomseed) = 0;

	virtual SapphireStorageError getLeaderboard(const SapphireUUID& leveluuid, const SapphireUUID& userid,
			SapphireLeaderboards leaderboardtype, unsigned int maxoutcount, ArrayList<LeaderboardEntry>* outentries, int* outuserindex,
			uint32* outuserscore, int32* outuserposition, PlayerDemoId* outuserdemoid, uint32* outtotalcount) = 0;
};

} // namespace userapp

#endif /* TEST_SAPPHIRE_SERVER_SAPPHIREDATASTORAGE_H_ */
