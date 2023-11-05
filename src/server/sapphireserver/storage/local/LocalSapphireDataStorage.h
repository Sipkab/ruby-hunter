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
 * LocalSapphireDataStorage.h
 *
 *  Created on: 2016. okt. 8.
 *      Author: sipka
 */

#ifndef TEST_SAPPHIRE_SERVER_STORAGE_LOCAL_LOCALSAPPHIREDATASTORAGE_H_
#define TEST_SAPPHIRE_SERVER_STORAGE_LOCAL_LOCALSAPPHIREDATASTORAGE_H_

#include <framework/io/files/StorageDirectoryDescriptor.h>
#include <framework/io/files/StorageFileDescriptor.h>
#include <framework/utils/ArrayList.h>
#include <framework/utils/LinkedList.h>
#include <framework/utils/ContainerLinkedNode.h>
#include <framework/random/RandomContext.h>
#include <framework/resource/Resource.h>
#include <framework/threading/Mutex.h>

#include <sapphire/level/SapphireLevelDescriptor.h>
#include <sapphireserver/storage/SapphireDataStorage.h>
#include <sapphire/level/Level.h>
#include <sapphire/community/SapphireUser.h>
#include <sapphire/level/SapphireUUID.h>
#include <sapphire/server/WorkerThread.h>
#include <sapphire/common/RegistrationToken.h>

namespace userapp {
using namespace rhfw;

class LocalSapphireDataStorage: public SapphireDataStorage {
	StorageDirectoryDescriptor usersDirectory { StorageDirectoryDescriptor::Root() + "users" };
	StorageDirectoryDescriptor levelsDirectory { StorageDirectoryDescriptor::Root() + "levels" };
	StorageDirectoryDescriptor messagesDirectory { StorageDirectoryDescriptor::Root() + "messages" };
	StorageDirectoryDescriptor hardwareDirectory { StorageDirectoryDescriptor::Root() + "hardware" };
	StorageDirectoryDescriptor demosDirectory { StorageDirectoryDescriptor::Root() + "demos" };
	unsigned int messagesFileIndex = 0;
	unsigned int currentMessagesFileMessageCount = 0;

	static int compareUUIDPtrs(const SapphireUUID* l, const SapphireUUID* r) {
		return l->compare(*r);
	}
	static int compareUUIDLPtrs(const SapphireUUID* l, const SapphireUUID& r) {
		return l->compare(r);
	}

	template<unsigned int Count = 64>
	class LockPool {
	public:
		Mutex mutexes[Count];
		LockPool() {
			for (Mutex& m : mutexes) {
				m.init();
			}
		}

		Mutex& operator[](const SapphireUUID& uuid) {
			return mutexes[uuid.lower64bit() % Count];
		}
		MutexLocker locker(const SapphireUUID& uuid) {
			return MutexLocker { (*this)[uuid] };
		}
	};

	class BuiltinLevelAssets {
	private:
		void fill(RAssetFile asset);
	public:
		class LevelAssetLink {
		public:
			static int compareUUIDs(const LevelAssetLink* l, const LevelAssetLink* r) {
				return l->levelUUID.compare(r->levelUUID);
			}
			static int compareAgainstUUID(const LevelAssetLink* l, const SapphireUUID& uuid) {
				return l->levelUUID.compare(uuid);
			}
			SapphireUUID levelUUID;
			RAssetFile asset;
		};
		ArrayList<LevelAssetLink> assetLinks;

		BuiltinLevelAssets();

		int getBuiltinLevelIndex(const SapphireUUID& uuid) const;

		LevelAssetLink** begin() {
			return assetLinks.begin();
		}
		LevelAssetLink** end() {
			return assetLinks.end();
		}
	};

	class StorageSapphireLevelDescriptor: public SapphireLevelDescriptor {
	public:
		using SapphireLevelDescriptor::SapphireLevelDescriptor;

		uint32 dateYear = 0;
		uint8 dateMonth = 0;
		uint8 dateDay = 0;

		uint32 ratingSum = 0;
		uint32 ratingCount = 0;

		void initDate(FileDescriptor& fd);
	};

	class StorageLevelRating {
	public:
		static int compareRatings(const StorageLevelRating* l, const StorageLevelRating* r) {
			return l->levelUUID.compare(r->levelUUID);
		}
		static int compareRatingsUUID(const StorageLevelRating* l, const SapphireUUID& uuid) {
			return l->levelUUID.compare(uuid);
		}

		SapphireUUID levelUUID;
		unsigned int rating;
	};
	class StorageSapphireUser {
	private:
		ArrayList<StorageLevelRating> levelRatings;
	public:
		static int compare(const StorageSapphireUser* l, const StorageSapphireUser* r) {
			return l->uuid.compare(r->uuid);
		}
		static int compareUUID(const StorageSapphireUser* l, const SapphireUUID& uuid) {
			return l->uuid.compare(uuid);
		}

		SapphireUUID uuid;
		RegistrationToken registrationToken;
		ArrayList<SapphireUUID> uploadedLevels;

		FixedString name;
		SapphireDifficulty difficultyColor = SapphireDifficulty::Tutorial;

		StorageSapphireUser() {
		}
		StorageSapphireUser(const SapphireUUID& uuid)
				: uuid(uuid) {
		}
		const SapphireUUID& getUUID() const {
			return uuid;
		}
		const RegistrationToken& getRegistrationToken() const {
			return registrationToken;
		}

		int findRatingIndex(const SapphireUUID& uuid) {
			return levelRatings.getIndexForSorted(uuid, StorageLevelRating::compareRatingsUUID);;
		}
		StorageLevelRating* findRating(const SapphireUUID& uuid) {
			int index = findRatingIndex(uuid);
			if (index < 0) {
				return nullptr;
			}
			return levelRatings.get(index);
		}

		unsigned int setRating(const SapphireUUID& leveluuid, unsigned int rating) {
			unsigned int oldrating = 0;
			int index = findRatingIndex(leveluuid);
			if (index < 0) {
				levelRatings.add(-index - 1, new StorageLevelRating { leveluuid, rating });
			} else {
				auto* storerating = levelRatings.get(index);
				oldrating = storerating->rating;
				storerating->rating = rating;
			}
			return oldrating;
		}

		void loadUploadedLevels(const FilePath& directory);
		void saveUploadedLevels(const FilePath& directory);
	};

	class StorageDiscussionMessage {
	public:
		StorageSapphireUser* user;
		FixedString message;
	};
	class StorageUserHardware {
		void loadProgress(FileDescriptor& fd);
	public:
		static int compare(const StorageUserHardware* l, const StorageUserHardware* r) {
			return l->hardwareUUID.compare(r->hardwareUUID);
		}
		static int compareUUID(const StorageUserHardware* l, const SapphireUUID& uuid) {
			return l->hardwareUUID.compare(uuid);
		}

		SapphireUUID hardwareUUID;
		ArrayList<SapphireUUID> seenLevels;
		ArrayList<SapphireUUID> finishedLevels;
		ProgressSynchId progressId = 0;
		HardwareProgressChangedListener::Events hardwareProgressChangedEvents;
		//lock on hardwareMutex to access hardwares
		ArrayList<AssociatedHardware> associatedHardwares;

		bool isSeenLevel(const SapphireUUID& uuid) {
			return seenLevels.getIndexForSorted(uuid, compareUUIDLPtrs) >= 0;
		}
		bool isFinishedLevel(const SapphireUUID& uuid) {
			return finishedLevels.getIndexForSorted(uuid, compareUUIDLPtrs) >= 0;
		}
		void loadProgress(const FilePath& directory);
		void saveAssociatedHardwares(const FilePath& directory);

		bool hasAssociatedHardware(const SapphireUUID& hardwareuuid) {
			return getAssociatedHardware(hardwareuuid) != nullptr;
		}

		AssociatedHardware* getAssociatedHardware(const SapphireUUID& hardwareuuid) {
			int idx = associatedHardwares.getIndexForSorted(hardwareuuid, AssociatedHardware::compareUUID);
			if (idx < 0) {
				return nullptr;
			}
			return associatedHardwares.get(idx);
		}
	};
	class StorageUserPosition {
	public:
		StorageSapphireUser* user = nullptr;
		int index = -1;
		StorageUserPosition() {
		}
		StorageUserPosition(StorageSapphireUser* user, int index)
				: user(user), index(index) {
		}
	};
	class StorageLeaderboardEntry {
	public:
		static int comparatorLeastSteps(StorageLeaderboardEntry* l, StorageLeaderboardEntry* r) {
			int res = (int) l->score - (int) r->score;
			return res == 0 ? l->user->uuid.compare(r->user->uuid) : res;
		}
		static int comparatorLeastTime(StorageLeaderboardEntry* l, StorageLeaderboardEntry* r) {
			int res = (int) l->score - (int) r->score;
			return res == 0 ? l->user->uuid.compare(r->user->uuid) : res;
		}
		static int comparatorMostGems(StorageLeaderboardEntry* l, StorageLeaderboardEntry* r) {
			//descending
			int res = (int) r->score - (int) l->score;
			return res == 0 ? l->user->uuid.compare(r->user->uuid) : res;
		}
		StorageSapphireUser* user;
		uint32 score;
		PlayerDemoId demoId;
		StorageUserPosition* position = nullptr;

		StorageLeaderboardEntry(StorageSapphireUser* user, uint32 score, PlayerDemoId demoid)
				: user(user), score(score), demoId(demoid) {
		}
	};
	class StorageLeaderboard {
		static int comparatorUserPositions(StorageUserPosition* l, StorageUserPosition* r) {
			return l->user->uuid.compare(r->user->uuid);
		}
	public:
		SapphireLeaderboards type;
		ArrayList<StorageLeaderboardEntry> entries;
		ArrayList<StorageUserPosition> userPositions;

		StorageLeaderboard(SapphireLeaderboards type)
				: type(type) {
		}

		template<typename Comparator>
		void addEntry(StorageLeaderboardEntry* entry, Comparator&& comparator) {
			StorageUserPosition userpos;
			userpos.user = entry->user;
			int userindex = userPositions.getIndexForSorted(&userpos, comparatorUserPositions);
			if (userindex >= 0) {
				//has previous entry, update it
				int toinsertindex = entries.getIndexForSorted(entry, util::forward<Comparator>(comparator));
				if (toinsertindex >= 0) {
					//has the same user uuid and score in the leaderboard already
					delete entry;
					return;
				}
				//score changed
				auto* gotpos = userPositions.get(userindex);
				auto* prev = entries.get(gotpos->index);
				if (comparator(prev, entry) < 0) {
					//previous entry has better score
					delete entry;
					return;
				}
				//remove previous entry
				delete entries.remove(gotpos->index);

				toinsertindex = -(toinsertindex + 1);
				if (gotpos->index < toinsertindex) {
					--toinsertindex;
				}
				entry->position = gotpos;
				entries.add(toinsertindex, entry);
				gotpos->index = toinsertindex;

				unsigned int size = entries.size();
				for (unsigned int i = toinsertindex + 1; i < size; ++i) {
					entries[i].position->index = i;
				}
			} else {
				int insertedindex = entries.setSorted(entry, util::forward<Comparator>(comparator));
				auto userpos = new StorageUserPosition(entry->user, insertedindex);
				entry->position = userpos;
				userPositions.add(-(userindex + 1), userpos);

				unsigned int size = entries.size();
				for (unsigned int i = insertedindex + 1; i < size; ++i) {
					entries[i].position->index = i;
				}
			}
		}
	};
	class StorageLevelStatistics {
	public:
		static int compareStats(StorageLevelStatistics* l, StorageLevelStatistics* r) {
			return l->levelUUID.compare(r->levelUUID);
		}
		static int compareStatsUUID(StorageLevelStatistics* l, const SapphireUUID& uuid) {
			return l->levelUUID.compare(uuid);
		}
		static const unsigned int DEMO_OFFSET_PARTITION = 8;
		SapphireUUID levelUUID;
		LevelStatistics stats;
		unsigned int playCount = 0;
		PlayerDemoId demoId = 0;
		LinkedList<StorageLeaderboard> leaderboards;
		/**
		 * Contains the file offset of a demo for every DEMO_OFFSET_PARTITIONth id
		 */
		ArrayList<uint64> playerDemoOffsets;

		StorageLevelStatistics(const SapphireUUID& leveluuid)
				: levelUUID(leveluuid) {
		}
		StorageLevelStatistics(const SapphireUUID& leveluuid, const LevelStatistics& stats, unsigned int playcount)
				: levelUUID(leveluuid), stats(stats), playCount(playcount) {
		}

		void addStats(const LevelStatistics& o) {
			stats += o;
			++playCount;
		}

		StorageLeaderboard& getLeaderboard(SapphireLeaderboards type) {
			for (auto&& l : leaderboards.objects()) {
				if (l.type == type) {
					return l;
				}
			}
			auto* res = new ContainerLinkedNode<StorageLeaderboard>(type);
			leaderboards.addToEnd(*res);
			return *res;
		}
		StorageLeaderboard* getLeaderboardIfExists(SapphireLeaderboards type) {
			for (auto l : leaderboards.pointers()) {
				if (l->type == type) {
					return l;
				}
			}
			return nullptr;
		}
	};

	Mutex levelsMutex { Mutex::auto_init { } };
	ArrayList<StorageSapphireLevelDescriptor> descriptors;

	Mutex usersMutex { Mutex::auto_init { } };
	ArrayList<StorageSapphireUser> users;
	LockPool<> usersLockPool;

	Mutex messagesMutex { Mutex::auto_init { } };
	ArrayList<StorageDiscussionMessage> messages;
	unsigned int messagesStartIndex = 0;

	Mutex hardwareMutex { Mutex::auto_init { } };
	ArrayList<StorageUserHardware> hardwares;
	LockPool<> hardwaresLockPool;

	Mutex statisticsMutex { Mutex::auto_init { } };
	ArrayList<StorageLevelStatistics> statistics;
	LockPool<> statisticsLevelLockPool;

	static void saveUser(StorageDirectoryDescriptor& dir, const StorageSapphireUser& user);
	bool loadUser(StorageDirectoryDescriptor& dir, StorageSapphireUser* user);

	unsigned int readMessagesFile(unsigned int index, bool* validfile, int formatnumber);
	unsigned int readMessagesFile1(unsigned int index, bool* validfile);
	unsigned int readMessagesFile2(unsigned int index, bool* validfile);

	template<typename EventsType, typename MutexType, typename ... Params>
	void broadcastListenerEvents(EventsType&& events, MutexType&& mutex, Params&&... params) {
		MutexLocker lock { util::forward<MutexType>(mutex) };
		for (auto&& l : util::forward<EventsType>(events).foreach()) {
			l(util::forward<Params>(params)...);
		}
	}
	template<typename EventsType, typename ... Params>
	void broadcastListenerEventsNoMutex(EventsType&& events, Params&&... params) {
		for (auto&& l : util::forward<EventsType>(events).foreach()) {
			l(util::forward<Params>(params)...);
		}
	}

	StorageSapphireUser* findUserLocked(const SapphireUUID& uuid);
	StorageSapphireUser* findUser(const SapphireUUID& uuid);
	StorageSapphireLevelDescriptor* findLevel(const SapphireUUID& uuid);
	StorageSapphireLevelDescriptor* findLevelLocked(const SapphireUUID& uuid);
	StorageUserHardware* findHardwareLocked(const SapphireUUID& uuid);
	StorageUserHardware* findHardware(const SapphireUUID& uuid);
	StorageUserHardware* getHardwareCreate(const SapphireUUID& uuid);
	StorageUserHardware* getHardwareCreateLocked(const SapphireUUID& uuid);
	int findLevelIndex(const SapphireUUID& uuid);
	int findLevelIndexLocked(const SapphireUUID& uuid);

	StorageLevelStatistics* findStatistics(const SapphireUUID& leveluuid);
	StorageLevelStatistics* findStatisticsLocked(const SapphireUUID& leveluuid);

	AutoResource<RandomContext> randomContext;
	Randomer* uuidRandomer = nullptr;

	Randomer* getRandomer();

	Mutex levelChangedListenersMutex { Mutex::auto_init { } };
	LevelChangedListener::Events levelChangedEvents;

	Mutex messagesChangedListenersMutex { Mutex::auto_init { } };
	MessagesChangedListener::Events messagesChangedEvents;

	WorkerThread messageWriterThread;

	HardwareAssociationListener::Events hardwareAssociationEvents;

	BuiltinLevelAssets builtinAssets;

	bool getBuiltinLevel(const SapphireUUID& uuid, Level* outlevel);

	void queryAssociatedHardwaresLocked(StorageUserHardware& hardware, ArrayList<AssociatedHardware>& outids);
	SapphireStorageError addHardwareAssociationLocked(StorageUserHardware& targethardware, StorageUserHardware& association,
			ArrayList<AssociatedHardware>& group);

	SapphireStorageError updateUserInfoLocked(StorageSapphireUser* user, const FixedString& name, SapphireDifficulty diffcolor);

	void initLevelData(const Level& level);

	void applyLeaderboardData(StorageSapphireUser* user, StorageLevelStatistics* foundstats, const LevelStatistics& stats,
			PlayerDemoId demoid, unsigned int demotime);
	void applyLeaderboardData(StorageSapphireUser* user, StorageLevelStatistics* foundstats,
				PlayerDemoId demoid, unsigned int gemWorth, unsigned int moveCount, unsigned int demotime);
public:
	LocalSapphireDataStorage();
	~LocalSapphireDataStorage();

	virtual SapphireStorageError queryLevels(SapphireLevelDetails* details, unsigned int maxcount, unsigned int start,
			unsigned int *outcount, const SapphireUUID& user) override;
	virtual SapphireStorageError queryMessages(SapphireDiscussionMessage* messages, unsigned int maxcount, unsigned int* inoutstart,
			unsigned int* outcount) override;
	virtual SapphireStorageError queryAssociatedHardwares(const SapphireUUID& hardwareuuid, ArrayList<AssociatedHardware>& outids) override;
	virtual SapphireStorageError queryLevelProgress(ProgressSynchId* progressid, const SapphireUUID& hardware, SapphireUUID* outlevel,
			SapphireLevelProgress* outprogress) override;

	virtual SapphireStorageError createHardwareAssociation(const SapphireUUID& hardware1, const SapphireUUID& hardware2) override;

	virtual SapphireStorageError getLevel(const SapphireUUID& uuid, Level* outlevel, bool* outisbuiltin) override;

	virtual SapphireStorageError saveLevel(const Level& level, const SapphireUUID& author) override;
	virtual SapphireStorageError removeLevel(const SapphireUUID& leveluuid) override;

	virtual SapphireStorageError loginUser(const SapphireUUID& uuid, RegistrationToken* outtoken, FixedString* outname,
			SapphireDifficulty* outdiffcolor) override;
	virtual SapphireStorageError registerUser(const SapphireUUID& uuid, const RegistrationToken& token) override;
	virtual SapphireStorageError updateUserInfo(const SapphireUUID& uuid, const FixedString& name, SapphireDifficulty diffcolor) override;

	virtual SapphireStorageError rateLevel(const SapphireUUID& userid, const SapphireUUID& leveluuid, unsigned int rating) override;

	virtual SapphireStorageError appendMessage(const SapphireUUID& userid, const char* message) override;

	virtual SapphireStorageError setLevelProgress(ProgressSynchId* progressid, const SapphireUUID& hardware, const SapphireUUID& level,
			SapphireLevelProgress progress) override;
	virtual SapphireStorageError getProgressId(const SapphireUUID& hardware, ProgressSynchId* outprogressid) override;

	virtual SapphireStorageError getAssociatedHardwareProgressId(const SapphireUUID& hardware, const SapphireUUID& associatedhardware,
			ProgressSynchId* outprogressid) override;
	virtual SapphireStorageError increaseAssociatedHardwareProgressId(const SapphireUUID& hardware, const SapphireUUID& associatedhardware,
			ProgressSynchId* progressid) override;

	virtual SapphireStorageError addLevelChangedListener(LevelChangedListener& listener) override;
	virtual SapphireStorageError removeLevelChangedListener(LevelChangedListener& listener) override;

	virtual SapphireStorageError addMessagesChangedListener(MessagesChangedListener& listener) override;
	virtual SapphireStorageError removeMessagesChangedListener(MessagesChangedListener& listener) override;

	virtual SapphireStorageError addHardwareProgressChangedListener(const SapphireUUID& hardware, HardwareProgressChangedListener& listener)
			override;
	virtual SapphireStorageError removeHardwareProgressChangedListener(const SapphireUUID& hardware,
			HardwareProgressChangedListener& listener) override;

	virtual SapphireStorageError addHardwareAssociationListener(HardwareAssociationListener& listener) override;
	virtual SapphireStorageError removeHardwareAssociationListener(HardwareAssociationListener& listener) override;

	virtual SapphireStorageError getLevelStatistics(const SapphireUUID& leveluuid, LevelStatistics* outstats, unsigned int* outplaycount)
			override;
	virtual SapphireStorageError appendLevelStatistics(const SapphireUUID& leveluuid, const SapphireUUID& userid, const FixedString& steps,
			uint32 randomseed) override;

	virtual SapphireStorageError getPlayerDemo(const SapphireUUID& leveluuid, PlayerDemoId demoid, FixedString* outsteps,
			uint32* outrandomseed) override;

	virtual SapphireStorageError getLeaderboard(const SapphireUUID& leveluuid, const SapphireUUID& userid,
			SapphireLeaderboards leaderboardtype, unsigned int maxoutcount, ArrayList<LeaderboardEntry>* outentries, int* outuserindex,
			uint32* outuserscore, int32* outuserposition, PlayerDemoId* outuserdemoid, uint32* outtotalcount) override;
};

} // namespace userapp

#endif /* TEST_SAPPHIRE_SERVER_STORAGE_LOCAL_LOCALSAPPHIREDATASTORAGE_H_ */
