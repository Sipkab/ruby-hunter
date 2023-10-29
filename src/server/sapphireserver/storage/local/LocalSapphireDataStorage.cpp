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
 * LocalSapphireDataStorage.cpp
 *
 *  Created on: 2016. okt. 8.
 *      Author: sipka
 */

#include <sapphireserver/storage/local/LocalSapphireDataStorage.h>
#include <sapphire/level/SapphireLevelDescriptor.h>
#include <sapphire/server/SapphireLevelDetails.h>
#include <sapphire/community/SapphireDiscussionMessage.h>

#include <framework/io/stream/OutputStream.h>
#include <framework/io/stream/InputStream.h>
#include <framework/io/stream/BufferedInputStream.h>
#include <framework/io/files/AssetFileDescriptor.h>

#include <sapphire/sapphireconstants.h>

#include <time.h>

#include <gen/assets.h>
#include <sapphireserver/servermain.h>

#define FILENAME_HARDWARE_PROGRESS "progress"
#define FILENAME_ASSOCIATED_HARDWARES "associated"

#define FILENAME_USER_UPLOADED_LEVELS "uplevels"

namespace userapp {
using namespace rhfw;

#define SERVER_RELEASE_VERSION 1
#define USER_DATA_FILENAME "usr.dat"
#define USER_RATINGS_FILENAME "usr.rating"
#define MESSAGES_FILE_FORMAT_FILENAME "messages.%x"
#define MESSAGES_FILE_FORMAT_2_FILENAME "messages2.%x"
#define MAX_MESSAGES_PER_FILE 16384
#define MAX_MESSAGE_CACHE_SIZE (MAX_MESSAGES_PER_FILE * 2)

static SapphireLevelCommProgress PROGRESS_COMM_MAP[(unsigned int) SapphireLevelProgress::MAX_VALUE + 1] { //
SapphireLevelCommProgress::Seen, // LEVEL_SEEN
		SapphireLevelCommProgress::Finished, // LEVEL_FINISHED
};
static SapphireLevelProgress COMM_PROGRESS_MAP[(unsigned int) SapphireLevelCommProgress::COMM_COUNT] { SapphireLevelProgress::LEVEL_SEEN, // Unknown
		SapphireLevelProgress::LEVEL_SEEN, // Seen
		SapphireLevelProgress::LEVEL_FINISHED, // Finished
		SapphireLevelProgress::LEVEL_SEEN, // Seen_NoSteps
		SapphireLevelProgress::LEVEL_FINISHED, // Finished_NoSteps
		SapphireLevelProgress::LEVEL_SEEN, // TimePlayed
		SapphireLevelProgress::LEVEL_SEEN, // IdOverride
};

SapphireLevelCommProgress sapphireLevelProgressToComm(SapphireLevelProgress progress) {
	if (progress > SapphireLevelProgress::MAX_VALUE) {
		return SapphireLevelCommProgress::Seen;
	}
	return PROGRESS_COMM_MAP[(uint32) progress];
}
SapphireLevelProgress sapphireLevelCommProgressToSapphireLevelProgress(SapphireLevelCommProgress progress) {
	if (progress >= SapphireLevelCommProgress::COMM_COUNT) {
		return SapphireLevelProgress::LEVEL_SEEN;
	}
	return COMM_PROGRESS_MAP[(uint32) progress];
}

int LocalSapphireDataStorage::BuiltinLevelAssets::getBuiltinLevelIndex(const SapphireUUID& uuid) const {
	return assetLinks.getIndexForSorted(uuid, LevelAssetLink::compareAgainstUUID);
}
void LocalSapphireDataStorage::BuiltinLevelAssets::fill(RAssetFile asset) {
	AssetFileDescriptor fd { asset };
	SapphireLevelDescriptor desc;
	if (SapphireLevelDescriptor::make(&desc, fd)) {
		assetLinks.setSorted(new LevelAssetLink { desc.uuid, asset }, LevelAssetLink::compareUUIDs);
	}
}

LocalSapphireDataStorage::BuiltinLevelAssets::BuiltinLevelAssets() {
	LOGI()<< "Loading builtin single classic levels...";
	for (auto&& asset : RAssets::gameres::game_sapphire::levels::enumerate()) {
		fill(asset);
	}
	LOGI() << "Loading builtin single expansion levels...";
	for (auto&& asset : RAssets::gameres::game_sapphire::levels::custom::enumerate()) {
		fill(asset);
	}
	LOGI() << "Loading builtin dual classi levels...";
	for (auto&& asset : RAssets::gameres::game_sapphire::levels::twoplayer::enumerate()) {
		fill(asset);
	}
	LOGI() << "Loading builtin dual expansion levels...";
	for (auto&& asset : RAssets::gameres::game_sapphire::levels::twoplayer::custom::enumerate()) {
		fill(asset);
	}
	for (auto&& asset : RAssets::gameres::game_sapphire::levels::elliotclassic::enumerate()) {
		fill(asset);
	}
	LOGI() << "Loading builtin levels done. " << assetLinks.size();
}

bool LocalSapphireDataStorage::getBuiltinLevel(const SapphireUUID& uuid, Level* outlevel) {
	int idx = builtinAssets.getBuiltinLevelIndex(uuid);
	if (idx < 0) {
		return false;
	}
	AssetFileDescriptor fd { builtinAssets.assetLinks.get(idx)->asset };
	return outlevel->loadLevel(fd);
}

void LocalSapphireDataStorage::StorageSapphireLevelDescriptor::initDate(FileDescriptor& fd) {
	long long modified = fd.lastModified();
	if (modified < 0) {
		LOGI()<< "Negative modification time for file descriptor";
		modified = 0;
	}
	time_t timeval = modified / 1000;
	struct tm* time = localtime(&timeval);
	ASSERT(time != nullptr) << "Failed: " << errno;
	if (time != nullptr) {
		dateYear = time->tm_year + 1900;
		dateMonth = time->tm_mon + 1;
		dateDay = time->tm_mday;
	}
}

LocalSapphireDataStorage::LocalSapphireDataStorage() {
	usersDirectory.create();
	levelsDirectory.create();
	messagesDirectory.create();
	hardwareDirectory.create();
	demosDirectory.create();

	postLogEvent("Loading community levels...");
	auto&& levelspath = levelsDirectory.getPath();
	for (auto&& file : levelsDirectory.enumerate()) {
		if (file.isDirectory()) {
			continue;
		}
		StorageFileDescriptor fd { levelspath + file };
		StorageSapphireLevelDescriptor desc;
		if (StorageSapphireLevelDescriptor::make(&desc, fd)) {
			desc.initDate(fd);

			desc.serverSideAvailable = true;
			desc.setFileDescriptor(new StorageFileDescriptor(util::move(fd)));
			descriptors.add(new StorageSapphireLevelDescriptor(util::move(desc)));
		} else {
			postLogEvent(FixedString {"Failed to make community level descriptor of file: "} + (const char*)file);
		}
	}
	postLogEvent("Loading users...");
	auto&& userspath = usersDirectory.getPath();
	for (auto&& dir : usersDirectory.enumerate()) {
		if(!dir.isDirectory()){
			continue;
		}
		StorageSapphireUser* user = new StorageSapphireUser();
		StorageDirectoryDescriptor userdir { userspath + dir };
		if (!loadUser(userdir, user)) {
			postLogEvent("Failed to load user.");
			delete user;
		} else {
			users.setSorted(user, StorageSapphireUser::compare);
		}
	}
	postLogEvent("Loading hardwares...");
	auto&& hardwarepath = hardwareDirectory.getPath();
	for (auto&& dir : hardwareDirectory.enumerate()) {
		if (!dir.isDirectory()) {
			continue;
		}
		StorageUserHardware* hardware = new StorageUserHardware();
		if (!SapphireUUID::fromString(&hardware->hardwareUUID, ((FilePath) dir).getURI())) {
			delete hardware;
			continue;
		}
		hardware->loadProgress(hardwarepath + dir);

		hardwares.setSorted(hardware, StorageUserHardware::compare);
	}

	postLogEvent("Loading builtin level demos to statistics and leaderboards...");
	for (auto&& a : builtinAssets) {
		AssetFileDescriptor fd { a->asset };
		Level l;
		if (l.loadLevel(fd)) {
			initLevelData(l);
		} else {
			char buf[256];
			snprintf(buf, sizeof(buf), "Failed to load level: %u", a->asset);
			postLogEvent(buf);
		}
	}
	postLogEvent("Loading user level demos to statistics and leaderboards...");
	for (auto&& l : descriptors) {
		Level level;
		if (level.loadLevel(l->getFileDescriptor())) {
			initLevelData(level);
		} else {
			postLogEvent(FixedString { "Failed to load level: " } + l->uuid);
		}
	}

	postLogEvent("Loading messages...");
	const unsigned int MAX_MESSAGES_FILE_READ_COUNT = 2;
	int messagesformat[MAX_MESSAGES_FILE_READ_COUNT] = { 1, 1 };
	int messagesindexes[MAX_MESSAGES_FILE_READ_COUNT] = { -1, -1 };
	int foundmessagesfiles = 0;
	while (true) {
		char buf[64];
		snprintf(buf, sizeof(buf), MESSAGES_FILE_FORMAT_FILENAME, messagesFileIndex);
		StorageFileDescriptor fd { messagesDirectory.getPath() + buf };
		if (fd.exists()) {
			messagesindexes[0] = messagesindexes[1];
			messagesindexes[1] = messagesFileIndex;

			++messagesFileIndex;
			++foundmessagesfiles;
		} else {
			break;
		}
	}
	messagesFileIndex = 0;
	postLogEvent("Loading messages 2...");
	while (true) {
		char buf[64];
		snprintf(buf, sizeof(buf), MESSAGES_FILE_FORMAT_2_FILENAME, messagesFileIndex);
		StorageFileDescriptor fd { messagesDirectory.getPath() + buf };
		if (fd.exists()) {
			messagesindexes[0] = messagesindexes[1];
			messagesindexes[1] = messagesFileIndex;

			messagesformat[0] = messagesformat[1];
			messagesformat[1] = 2;

			++messagesFileIndex;
			++foundmessagesfiles;
		} else {
			break;
		}
	}

	if (foundmessagesfiles > 0) {
		if (foundmessagesfiles > MAX_MESSAGES_FILE_READ_COUNT) {
			//TODO update server code
			foundmessagesfiles = MAX_MESSAGES_FILE_READ_COUNT;
		}
		//messagesFileIndex is the last non-existing file
		//read the last and before that
		bool validfile;
		unsigned int padding = MAX_MESSAGES_FILE_READ_COUNT - foundmessagesfiles;
		for (unsigned int i = 0; i < foundmessagesfiles; ++i) {
			currentMessagesFileMessageCount = readMessagesFile(messagesindexes[padding + i], &validfile, messagesformat[padding + i]);
		}
		if (messagesformat[foundmessagesfiles - 1] == 1) {
			//last file is format 1, the new one should be format 2, reset file index
			messagesFileIndex = 0;
		} else {
			messagesFileIndex = messagesformat[foundmessagesfiles - 1];
		}
		if (!validfile) {
			++messagesFileIndex;
			currentMessagesFileMessageCount = 0;
		}
	} else {
		messagesFileIndex = 0;
		currentMessagesFileMessageCount = 0;
	}

	messageWriterThread.start();
}
LocalSapphireDataStorage::~LocalSapphireDataStorage() {
	messageWriterThread.stop();
	delete uuidRandomer;
}
void LocalSapphireDataStorage::initLevelData(const Level& level) {
	auto&& uuid = level.getInfo().uuid;

	StorageFileDescriptor dfd { demosDirectory.getPath() + (const char*) uuid.asString() };
	if (!dfd.exists()) {
		//no demos file, ignore
		return;
	}
	postLogEvent(FixedString { "Load level demos: " } + uuid.asString() + " : " + level.getInfo().title);

	bool fixdemofile = false;
	long long pos = 0;
	long long size = dfd.size();
	{
		auto&& istream = EndianInputStream<Endianness::Big>::wrap(dfd.openInputStream());

		SapphireUUID userid;
		uint32 randomseed;
		FixedString steps;

		auto* foundstats = findStatisticsLocked(uuid);

		unsigned int democount = 0;
		unsigned int unsuccessfulcount = 0;

		for (;;++democount) {
			pos = istream.getPosition();
			if (!istream.deserialize<SapphireUUID>(userid)
					|| !istream.deserialize<uint32>(randomseed) || !istream.deserialize<FixedString>(steps)) {
				//end of stream probably
				if (size != pos) {
					postLogEvent(FixedString { "Failed to deserialize demo data: " } + uuid.asString() + " at demo index: " + FixedString::toString(democount) + " file pos: " + FixedString::toString(pos) + " file size: " + FixedString::toString(size));

					//probably some write failed last time, when the server ran out of space
					//fix the descriptor by removing the trailing
					//do it below after we close the stream
					fixdemofile = true;
				}
				break;
			}

			//only add the statistics when at least one demo is successfully loaded
			if (foundstats == nullptr) {
				foundstats = new StorageLevelStatistics(uuid);
				statistics.setSorted(foundstats, StorageLevelStatistics::compareStats);
			}

			Level played = level;
			played.setRandomSeed(randomseed);
			DemoPlayer::playMovesUntilSuccess(steps, steps.length() / played.getPlayerCount(), played);
			ASSERT(played.isSuccessfullyOver());
			if (!played.isSuccessfullyOver()) {
				postLogEvent(FixedString { "Level demo is not successful: " } + uuid.asString() + " by " + userid.asString() + " at demo index: " + FixedString::toString(democount) + " file pos: " + FixedString::toString(pos));
				++unsuccessfulcount;
				continue;
			}
			auto* user = findUserLocked(userid);
			ASSERT(user != nullptr) << "referenced user is missing from demo file";
			if (user == nullptr) {
				postLogEvent(FixedString { "User not found from demo file: " } + userid.asString());
			}

			foundstats->addStats(played.getStatistics());
			if (user != nullptr) {
				//the user might've not been found, don't apply the leaderboard then
				applyLeaderboardData(user, foundstats, played.getStatistics(), foundstats->demoId, played.getTurn());
			}

			if (foundstats->demoId % StorageLevelStatistics::DEMO_OFFSET_PARTITION == 0) {
				foundstats->playerDemoOffsets.add(new uint64(pos));
			}

			foundstats->demoId++;
		}

		postLogEvent(FixedString { "Level demo loading done: " } + uuid.asString() + " DemoCount: " + FixedString::toString(democount) + " Unsuccessful: " + FixedString::toString(unsuccessfulcount));
	}
	if (fixdemofile) {
		postLogEvent(FixedString { "Fixing demo file: " } + uuid.asString());
		StorageFileDescriptor backupfd { demosDirectory.getPath() + (const char*) (uuid.asString() + "_backup_" + FixedString::toString(size)) };
		if (!dfd.move(backupfd)) {
			postLogEvent(FixedString { "Failed to move demo file for repair: " } + uuid.asString());
		} else {
			{
				auto&& istream = EndianInputStream<Endianness::Big>::wrap(backupfd.openInputStream());
				auto&& ostream = EndianOutputStream<Endianness::Big>::wrap(dfd.openOutputStream());
				char buffer[4096];
				for (long long copied = 0; copied < pos;) {
					long long remain = pos - copied;
					unsigned int rc = (unsigned int) (sizeof(buffer) < remain ? sizeof(buffer) : remain);
					int read = istream.read(buffer, rc);
					if (read <= 0) {
						postLogEvent(FixedString { "Failed to read bytes for demo repair: " } + uuid.asString() + " to read: " + FixedString::toString(rc) + " actually read: " + FixedString::toString(read));
						break;
					}
					ostream.write(buffer, read);
					copied += read;
				}
			}
			long long nsize = dfd.size();
			if (nsize != pos) {
				postLogEvent(FixedString { "Failed to repair demo data: " } + uuid.asString() + " from pos: " + FixedString::toString(pos) + " new file size: " + FixedString::toString(nsize));
			}
		}
	}

}
unsigned int LocalSapphireDataStorage::readMessagesFile(unsigned int index, bool* validfile, int formatnumber) {
	switch (formatnumber) {
		case 1: {
			return readMessagesFile1(index, validfile);
		}
		case 2: {
			return readMessagesFile2(index, validfile);
		}
		default: {
			THROW()<< formatnumber;
			break;
		}
	}
	return 0;
}
unsigned int LocalSapphireDataStorage::readMessagesFile1(unsigned int index, bool* validfile) {
	*validfile = true;
	char buf[64];
	snprintf(buf, sizeof(buf), MESSAGES_FILE_FORMAT_FILENAME, index);
	StorageFileDescriptor fd { messagesDirectory.getPath() + buf };
	LOGI()<< "Read messages file: " << fd.getPath().getURI();
	auto istream = EndianInputStream<Endianness::Big>::wrap(BufferedInputStream::wrap(fd.openInputStream()));
	unsigned int result = 0;
	while (true) {
		SapphireUUID userid;
		FixedString message;
		FixedString username;
		SapphireDifficulty diffcolor;
		if (istream.deserialize<SapphireUUID>(userid)) {
			if (!istream.deserialize<SafeFixedString<SAPPHIRE_USERNAME_MAX_LEN>>(username)
					|| !istream.deserialize<SafeFixedString<SAPPHIRE_DISCUSSION_MESSAGE_MAX_LEN>>(message)
					|| !istream.deserialize<SapphireDifficulty>(diffcolor)) {
				//file is corrupted
				*validfile = false;
				break;
			}
		} else {
			break;
		}

		auto* founduser = findUserLocked(userid);
		if (founduser == nullptr) {
			//user not found anymore, file is corrupted
			*validfile = false;
			LOGI()<< "User not found in messages file: " << fd.getPath().getURI() << " " << userid.asString();
			break;
		}

		auto* msg = new StorageDiscussionMessage { founduser, util::move(message) };
		messages.add(msg);
		++result;

		if (founduser->name == nullptr || founduser->difficultyColor < diffcolor) {
			updateUserInfoLocked(founduser, username, diffcolor);
		}
	}
	return result;
}
unsigned int LocalSapphireDataStorage::readMessagesFile2(unsigned int index, bool* validfile) {
	*validfile = true;
	char buf[64];
	snprintf(buf, sizeof(buf), MESSAGES_FILE_FORMAT_2_FILENAME, index);
	StorageFileDescriptor fd { messagesDirectory.getPath() + buf };
	LOGI()<< "Read messages file: " << fd.getPath().getURI();
	auto istream = EndianInputStream<Endianness::Big>::wrap(BufferedInputStream::wrap(fd.openInputStream()));
	unsigned int result = 0;
	while (true) {
		SapphireUUID userid;
		FixedString message;
		FixedString username;
		SapphireDifficulty diffcolor;
		if (istream.deserialize<SapphireUUID>(userid)) {
			if (!istream.deserialize<SafeFixedString<SAPPHIRE_DISCUSSION_MESSAGE_MAX_LEN>>(message)) {
				//file is corrupted
				*validfile = false;
				break;
			}
		} else {
			break;
		}

		auto* founduser = findUserLocked(userid);
		if (founduser == nullptr) {
			//user not found anymore, file is corrupted
			*validfile = false;
			LOGI()<< "User not found in messages file: " << fd.getPath().getURI() << " " << userid.asString();
			break;
		}

		auto* msg = new StorageDiscussionMessage { founduser, util::move(message) };
		messages.add(msg);
		++result;
	}
	return result;
}

SapphireStorageError LocalSapphireDataStorage::queryLevels(SapphireLevelDetails* details, unsigned int maxcount, unsigned int start,
		unsigned int *outcount, const SapphireUUID& user) {
	*outcount = 0;
	if (maxcount == 0) {
		return SapphireStorageError::SUCCESS;
	}
	auto* storeuser = findUser(user);
	if (storeuser == nullptr) {
		return SapphireStorageError::INVALID_USER_UUID;
	}
	MutexLocker l { levelsMutex };
	if (start >= this->descriptors.size()) {
		return SapphireStorageError::OUT_OF_BOUNDS;
	}

	MutexLocker ul = usersLockPool.locker(user);
	for (unsigned int i = start; i < descriptors.size() && *outcount < maxcount; ++i, ++(*outcount)) {
		auto& det = *details++;
		auto& d = descriptors[i];

		det = d;

		det.dateYear = d.dateYear;
		det.dateMonth = d.dateMonth;
		det.dateDay = d.dateDay;

		det.ratingSum = d.ratingSum;
		det.ratingCount = d.ratingCount;

		auto* rating = storeuser->findRating(d.uuid);
		if (rating != nullptr) {
			det.userRating = rating->rating;
		}
	}
	return SapphireStorageError::SUCCESS;
}
SapphireStorageError LocalSapphireDataStorage::appendMessage(const SapphireUUID& userid, const char* message) {
	if (message == nullptr) {
		return SapphireStorageError::NULLPOINTER;
	}
	auto* user = findUser(userid);
	if (user == nullptr) {
		return SapphireStorageError::INVALID_USER_UUID;
	}
	FixedString msgstr { message };
	SapphireUUID useruuid = userid;
	if (msgstr.length() > SAPPHIRE_DISCUSSION_MESSAGE_MAX_LEN) {
		return SapphireStorageError::OUT_OF_BOUNDS;
	}
	auto* msg = new StorageDiscussionMessage { user, message };
	{
		MutexLocker lock { messagesMutex };
		messages.add(msg);
		if (messages.size() > MAX_MESSAGE_CACHE_SIZE) {
			delete messages.remove(0);
			++messagesStartIndex;
		}
	}
	messageWriterThread.post([=] {
		if(currentMessagesFileMessageCount >= MAX_MESSAGES_PER_FILE) {
			++messagesFileIndex;
			currentMessagesFileMessageCount = 0;
		} else {
			++currentMessagesFileMessageCount;
		}
		char buf[64];
		snprintf(buf, sizeof(buf), MESSAGES_FILE_FORMAT_2_FILENAME, messagesFileIndex);
		StorageFileDescriptor fd {messagesDirectory.getPath() + buf};

		auto ostream = EndianOutputStream<Endianness::Big>::wrap(fd.openAppendStream());
		ostream.serialize<SapphireUUID>(useruuid);
		ostream.serialize<FixedString>(msgstr);

	});
	broadcastListenerEvents(messagesChangedEvents, messagesChangedListenersMutex, messagesStartIndex, messages.size());

	return SapphireStorageError::SUCCESS;
}
SapphireStorageError LocalSapphireDataStorage::queryMessages(SapphireDiscussionMessage* messages, unsigned int maxcount,
		unsigned int* start, unsigned int* outcount) {
	MutexLocker lock { messagesMutex };

	if (maxcount == 0 || *start >= messagesStartIndex + this->messages.size()) {
		*start = messagesStartIndex;
		*outcount = this->messages.size();
		return SapphireStorageError::OUT_OF_BOUNDS;
	}

	*outcount = 0;
	if (*start < messagesStartIndex) {
		unsigned int diff = messagesStartIndex - *start;
		if (diff >= maxcount) {
			*start = messagesStartIndex;
			*outcount = this->messages.size();
			return SapphireStorageError::OUT_OF_BOUNDS;
		}
		*start += diff;
		maxcount -= diff;
	}
	if (maxcount == 0) {
		return SapphireStorageError::SUCCESS;
	}
	for (unsigned int i = *start; i < this->messages.size() && *outcount < maxcount; ++i, ++(*outcount)) {
		auto& msg = this->messages[i];
		auto& target = *messages++;

		target.message = msg.message;
		MutexLocker ml = usersLockPool.locker(msg.user->uuid);
		target.userName = msg.user->name;
		target.difficultyColor = msg.user->difficultyColor;
	}
	return SapphireStorageError::SUCCESS;
}
void LocalSapphireDataStorage::queryAssociatedHardwaresLocked(StorageUserHardware& hardware, ArrayList<AssociatedHardware>& outids) {
	outids.addAll(hardware.associatedHardwares);
}
SapphireStorageError LocalSapphireDataStorage::queryAssociatedHardwares(const SapphireUUID& hardwareuuid,
		ArrayList<AssociatedHardware>& outids) {
	auto* h = findHardware(hardwareuuid);
	if (h == nullptr) {
		return SapphireStorageError::INVALID_USER_UUID;
	}
	MutexLocker ml = hardwaresLockPool.locker(h->hardwareUUID);
	queryAssociatedHardwaresLocked(*h, outids);
	return SapphireStorageError::SUCCESS;
}
SapphireStorageError LocalSapphireDataStorage::queryLevelProgress(ProgressSynchId* progressid, const SapphireUUID& hardware,
		SapphireUUID* outlevel, SapphireLevelProgress* outprogress) {
	auto* h = findHardware(hardware);
	if (h == nullptr) {
		return SapphireStorageError::INVALID_HARDWARE;
	}
	MutexLocker ml = hardwaresLockPool.locker(h->hardwareUUID);
	if (*progressid >= h->progressId) {
		*progressid = h->progressId;
		return SapphireStorageError::OUT_OF_BOUNDS;
	}
	auto&& istream =
			EndianInputStream<Endianness::Big>::wrap(
					StorageFileDescriptor { hardwareDirectory.getPath() + (const char*) hardware.asString() + FILENAME_HARDWARE_PROGRESS }.openInputStream());
	long long toseek = *progressid * (16 + 4);
	if (istream.seek(toseek, SeekMethod::BEGIN) != toseek) {
		return SapphireStorageError::STORAGE_UNAVAILABLE;
	}
	if (istream.deserialize<SapphireUUID>(*outlevel) && istream.deserialize<uint32>(reinterpret_cast<uint32&>(*outprogress))) {
		return SapphireStorageError::SUCCESS;
	}
	return SapphireStorageError::STORAGE_UNAVAILABLE;
}
SapphireStorageError LocalSapphireDataStorage::createHardwareAssociation(const SapphireUUID& targethardware,
		const SapphireUUID& associatedhardware) {
	MutexLocker ml { hardwareMutex };
	MutexLocker tml = hardwaresLockPool.locker(targethardware);
	MutexLocker aml = hardwaresLockPool.locker(associatedhardware);
	auto* h = findHardwareLocked(targethardware);
	if (h == nullptr) {
		return SapphireStorageError::INVALID_HARDWARE;
	}
	if (h->hasAssociatedHardware(associatedhardware)) {
		return SapphireStorageError::HARDWARE_ALREADY_ASSOCIATED;
	}
	auto* toadd = findHardwareLocked(associatedhardware);
	if (toadd == nullptr) {
		return SapphireStorageError::INVALID_HARDWARE;
	}
	ArrayList<AssociatedHardware> othergroup { toadd->associatedHardwares };
	for (auto&& ah : h->associatedHardwares) {
		auto* ahh = findHardwareLocked(ah->hardwareUUID);
		ASSERT(ahh != nullptr) << ah->hardwareUUID.asString();

		addHardwareAssociationLocked(*ahh, *toadd, othergroup);
	}
	addHardwareAssociationLocked(*h, *toadd, othergroup);
	toadd->saveAssociatedHardwares(hardwareDirectory.getPath() + (const char*) toadd->hardwareUUID.asString());
	for (auto&& gh : othergroup) {
		auto* found = findHardwareLocked(gh->hardwareUUID);
		ASSERT(found != nullptr) << gh->hardwareUUID.asString();
		MutexLocker fml = hardwaresLockPool.locker(found->hardwareUUID);

		found->saveAssociatedHardwares(hardwareDirectory.getPath() + (const char*) found->hardwareUUID.asString());
	}
	return SapphireStorageError::SUCCESS;
}
SapphireStorageError LocalSapphireDataStorage::addHardwareAssociationLocked(StorageUserHardware& hardware1, StorageUserHardware& hardware2,
		ArrayList<AssociatedHardware>& group) {
	MutexLocker ml1 = hardwaresLockPool.locker(hardware1.hardwareUUID);
	{
		MutexLocker ml2 = hardwaresLockPool.locker(hardware2.hardwareUUID);
		ASSERT(!hardware1.hasAssociatedHardware(hardware2.hardwareUUID));
		ASSERT(!hardware2.hasAssociatedHardware(hardware1.hardwareUUID));
		auto* basea1 = new AssociatedHardware(hardware2.hardwareUUID, 0);
		auto* basea2 = new AssociatedHardware(hardware1.hardwareUUID, 0);
		hardware1.associatedHardwares.setSorted(basea1, AssociatedHardware::compare);
		hardware2.associatedHardwares.setSorted(basea2, AssociatedHardware::compare);

		broadcastListenerEventsNoMutex(hardwareAssociationEvents, *basea1, *basea2, true);
	}
	for (auto&& gh : group) {
		auto* found = findHardwareLocked(gh->hardwareUUID);
		ASSERT(found != nullptr);
		ASSERT(!hardware1.hasAssociatedHardware(gh->hardwareUUID));
		ASSERT(!found->hasAssociatedHardware(hardware1.hardwareUUID));

		MutexLocker ml1 = hardwaresLockPool.locker(found->hardwareUUID);

		auto* a1 = new AssociatedHardware(gh->hardwareUUID, 0);
		auto* a2 = new AssociatedHardware(hardware1.hardwareUUID, 0);
		hardware1.associatedHardwares.setSorted(a1, AssociatedHardware::compare);
		found->associatedHardwares.setSorted(a2, AssociatedHardware::compare);

		broadcastListenerEventsNoMutex(hardwareAssociationEvents, *a1, *a2, true);
	}
	hardware1.saveAssociatedHardwares(hardwareDirectory.getPath() + (const char*) hardware1.hardwareUUID.asString());
	return SapphireStorageError::SUCCESS;
}

SapphireStorageError LocalSapphireDataStorage::getLevel(const SapphireUUID& uuid, Level* outlevel, bool* outisbuiltin) {
	if (!uuid) {
		return SapphireStorageError::LEVEL_NOT_FOUND;
	}
	if (getBuiltinLevel(uuid, outlevel)) {
		*outisbuiltin = true;
		return SapphireStorageError::SUCCESS;
	}
	StorageSapphireLevelDescriptor* desc = findLevel(uuid);
	if (desc == nullptr) {
		return SapphireStorageError::LEVEL_NOT_FOUND;
	}
	bool loadres = outlevel->loadLevel(desc->getFileDescriptor());
	if (loadres) {
		*outisbuiltin = false;
		return SapphireStorageError::SUCCESS;
	}
	return SapphireStorageError::LEVEL_FAILED_TO_LOAD;
}

SapphireStorageError LocalSapphireDataStorage::saveLevel(const Level& levelarg, const SapphireUUID& author) {
	if (!levelarg.getInfo().uuid) {
		return SapphireStorageError::LEVEL_ALREADY_EXISTS;
	}
	if (levelarg.getDemoCount() == 0) {
		return SapphireStorageError::NO_DEMO;
	}
	if (levelarg.getInfo().title.length() == 0) {
		return SapphireStorageError::NO_TITLE;
	}
	StorageSapphireUser* user = findUser(author);
	if (user == nullptr) {
		return SapphireStorageError::INVALID_USER_UUID;
	}

	bool demoremoved = false;
	Level level { levelarg };
	for (unsigned int i = 0; i < level.getDemoCount(); ++i) {
		Level test { level };
		DemoPlayer player;
		player.playFully(level.getDemo(i), test);
		if (!test.isSuccessfullyOver()) {
			level.removeDemo(i);
			--i;
			demoremoved = true;
		}
	}
	if (level.getDemoCount() == 0) {
		return SapphireStorageError::DEMO_INCORRECT;
	}
	StorageSapphireLevelDescriptor* desc;
	unsigned int index;
	{
		MutexLocker ml { levelsMutex };
		desc = findLevelLocked(level.getInfo().uuid);

		if (desc != nullptr) {
			return SapphireStorageError::LEVEL_ALREADY_EXISTS;
		} else {
			desc = new StorageSapphireLevelDescriptor(level);
			desc->setFileDescriptor(new StorageFileDescriptor(levelsDirectory.getPath() + (const char*) desc->uuid.asString()));
			desc->serverSideAvailable = true;
			index = descriptors.size();
			descriptors.add(desc);
		}

		level.saveLevel(desc->getFileDescriptor());
		desc->initDate(desc->getFileDescriptor());
	}
	{
		MutexLocker ul = usersLockPool.locker(user->uuid);
		user->uploadedLevels.setSorted(new SapphireUUID(desc->uuid), compareUUIDPtrs);
		user->saveUploadedLevels(usersDirectory.getPath() + (const char*) user->uuid.asString());
	}

	broadcastListenerEvents(levelChangedEvents, levelChangedListenersMutex, index, LevelChangeInfo::ADDED);
	return demoremoved ? SapphireStorageError::LEVEL_SAVE_SUCCESS_DEMO_REMOVED : SapphireStorageError::SUCCESS;
}
//1:
//	uuid
//	reg token
//2:
//	name
//	color
#define STORAGE_USER_FORMAT 2
void LocalSapphireDataStorage::saveUser(StorageDirectoryDescriptor& dir, const StorageSapphireUser& user) {
	LOGI()<< "Save user: " << user.getUUID().asString();
	StorageFileDescriptor fd {dir.getPath() + USER_DATA_FILENAME};
	auto stream = EndianOutputStream<Endianness::Big>::wrap(fd.openOutputStream());

	stream.serialize<uint32>(STORAGE_USER_FORMAT);
	stream.serialize<SapphireUUID>(user.getUUID());
	stream.serialize<RegistrationToken>(user.getRegistrationToken());
	stream.serialize<FixedString>(user.name);
	stream.serialize<SapphireDifficulty>(user.difficultyColor);
}

bool LocalSapphireDataStorage::loadUser(StorageDirectoryDescriptor& dir, StorageSapphireUser* user) {
	{
		StorageFileDescriptor fd { dir.getPath() + USER_DATA_FILENAME };
		auto stream = EndianInputStream<Endianness::Big>::wrap(BufferedInputStream::wrap(fd.openInputStream()));

		uint32 version;
		if (!stream.deserialize<uint32>(version)) {
			return false;
		}
		switch (version) {
			case 1: {
				if (!stream.deserialize<SapphireUUID>(user->uuid) || !stream.deserialize<RegistrationToken>(user->registrationToken)) {
					LOGI()<< "Failed to read user data " << fd.getPath().getURI();
					return false;
				}
				break;
			}
			case 2: {
				if (!stream.deserialize<SapphireUUID>(user->uuid) || !stream.deserialize<RegistrationToken>(user->registrationToken)
						|| !stream.deserialize<SafeFixedString<SAPPHIRE_USERNAME_MAX_LEN>>(user->name)
						|| !stream.deserialize<SapphireDifficulty>(user->difficultyColor)) {
					LOGI() << "Failed to read user data " << fd.getPath().getURI();
					return false;
				}
				break;
			}
			default: {
				LOGE() << "Unknown " USER_DATA_FILENAME " version: " << version;
				return false;
			}
		}
	}
	{
		StorageFileDescriptor fd {dir.getPath() + USER_RATINGS_FILENAME};
		auto stream = EndianInputStream<Endianness::Big>::wrap(BufferedInputStream::wrap(fd.openInputStream()));
		while (true) {
			SapphireUUID leveluuid;
			uint8 rating;
			if (!stream.deserialize<SapphireUUID>(leveluuid) || !stream.deserialize<uint8>(rating)) {
				break;
			}
			auto* level = findLevel(leveluuid);
			if (level != nullptr) {
				unsigned int oldrating = user->setRating(leveluuid, rating);
				level->ratingSum += rating - oldrating;
				if (oldrating == 0) {
					++(level->ratingCount);
				}
			}
		}
	}
	user->loadUploadedLevels(dir.getPath());

	return true;
}

SapphireStorageError LocalSapphireDataStorage::removeLevel(const SapphireUUID& leveluuid) {
	//TODO
	StorageSapphireLevelDescriptor* desc = nullptr;
	int index = -1;
	{
		MutexLocker l { levelsMutex };
		for (int i = 0; i < descriptors.size(); ++i) {
			auto* d = &descriptors[i];
			if (d->uuid == leveluuid) {
				desc = d;
				index = i;
				break;
			}
		}
		if (desc == nullptr) {
			return SapphireStorageError::LEVEL_NOT_FOUND;
		}
		descriptors.remove(index);
	}
	//TODO don't remove the file but move it to some other location
	desc->getFileDescriptor().remove();
	delete desc;

	broadcastListenerEvents(levelChangedEvents, levelChangedListenersMutex, (unsigned int) index, LevelChangeInfo::REMOVED);
	return SapphireStorageError::SUCCESS;
}

LocalSapphireDataStorage::StorageSapphireUser* LocalSapphireDataStorage::findUserLocked(const SapphireUUID& uuid) {
	int idx = users.getIndexForSorted(uuid, StorageSapphireUser::compareUUID);
	if (idx < 0) {
		return nullptr;
	}
	return users.get(idx);
}
LocalSapphireDataStorage::StorageSapphireUser* LocalSapphireDataStorage::findUser(const SapphireUUID& uuid) {
	if (!uuid) {
		return nullptr;
	}
	MutexLocker l { usersMutex };
	return findUserLocked(uuid);
}
LocalSapphireDataStorage::StorageUserHardware* LocalSapphireDataStorage::findHardwareLocked(const SapphireUUID& uuid) {
	int idx = hardwares.getIndexForSorted(uuid, StorageUserHardware::compareUUID);
	if (idx < 0) {
		return nullptr;
	}
	return hardwares.get(idx);
}

LocalSapphireDataStorage::StorageUserHardware* LocalSapphireDataStorage::findHardware(const SapphireUUID& uuid) {
	if (!uuid) {
		return nullptr;
	}
	MutexLocker l { hardwareMutex };
	return findHardwareLocked(uuid);
}
LocalSapphireDataStorage::StorageUserHardware* LocalSapphireDataStorage::getHardwareCreate(const SapphireUUID& uuid) {
	MutexLocker l { hardwareMutex };
	return getHardwareCreateLocked(uuid);
}
LocalSapphireDataStorage::StorageUserHardware* LocalSapphireDataStorage::getHardwareCreateLocked(const SapphireUUID& uuid) {
	StorageUserHardware* h = findHardwareLocked(uuid);
	if (h != nullptr) {
		return h;
	}
	h = new StorageUserHardware();
	h->hardwareUUID = uuid;
	hardwares.setSorted(h, StorageUserHardware::compare);
	StorageDirectoryDescriptor dir { hardwareDirectory.getPath() + (const char*) uuid.asString() };
	dir.create();
	h->loadProgress(dir.getPath());
	return h;
}

Randomer* LocalSapphireDataStorage::getRandomer() {
	if (randomContext == nullptr) {
		randomContext = Resource<RandomContext> { new ResourceBlock { new RandomContext { } } };
		uuidRandomer = randomContext->createRandomer();
	}
	return uuidRandomer;
}

LocalSapphireDataStorage::StorageSapphireLevelDescriptor* LocalSapphireDataStorage::findLevel(const SapphireUUID& uuid) {
	MutexLocker l { levelsMutex };
	return findLevelLocked(uuid);
}
LocalSapphireDataStorage::StorageSapphireLevelDescriptor* LocalSapphireDataStorage::findLevelLocked(const SapphireUUID& uuid) {
	for (auto* d : descriptors) {
		if (d->uuid == uuid) {
			return d;
		}
	}
	return nullptr;
}
int LocalSapphireDataStorage::findLevelIndex(const SapphireUUID& uuid) {
	MutexLocker l { levelsMutex };
	return findLevelIndexLocked(uuid);
}
int LocalSapphireDataStorage::findLevelIndexLocked(const SapphireUUID& uuid) {
	for (int i = 0; i < descriptors.size(); ++i) {
		if (descriptors[i].uuid == uuid) {
			return i;
		}
	}
	return -1;
}

SapphireStorageError LocalSapphireDataStorage::rateLevel(const SapphireUUID& userid, const SapphireUUID& leveluuid, unsigned int rating) {
	if (rating < 1 || rating > 5) {
		return SapphireStorageError::INVALID_RATING;
	}
	auto* user = findUser(userid);
	if (user == nullptr) {
		return SapphireStorageError::INVALID_USER_UUID;
	}
	int levelindex;
	{
		MutexLocker l { levelsMutex };
		levelindex = findLevelIndex(leveluuid);
		if (levelindex < 0) {
			return SapphireStorageError::LEVEL_NOT_FOUND;
		}
		unsigned int oldrating;
		{
			MutexLocker ul = usersLockPool.locker(userid);
			oldrating = user->setRating(leveluuid, rating);
		}
		if (oldrating == rating) {
			return SapphireStorageError::SUCCESS;
		}
		auto* level = &descriptors[levelindex];

		level->ratingSum += rating - oldrating;
		if (oldrating == 0) {
			++(level->ratingCount);
		}
	}

	StorageFileDescriptor fd { usersDirectory.getPath() + (const char*) userid.asString() + USER_RATINGS_FILENAME };

	auto ostream = EndianOutputStream<Endianness::Host>::wrap(fd.openAppendStream());
	ostream.serialize<SapphireUUID>(leveluuid);
	ostream.serialize<uint8>(rating);

	broadcastListenerEvents(levelChangedEvents, levelChangedListenersMutex, (unsigned int) levelindex, LevelChangeInfo::RATING_CHANGED);

	return SapphireStorageError::SUCCESS;
}

SapphireStorageError LocalSapphireDataStorage::addLevelChangedListener(LevelChangedListener& listener) {
	MutexLocker lock { levelChangedListenersMutex };
	levelChangedEvents += listener;
	return SapphireStorageError::SUCCESS;
}
SapphireStorageError LocalSapphireDataStorage::removeLevelChangedListener(LevelChangedListener& listener) {
	MutexLocker lock { levelChangedListenersMutex };
	levelChangedEvents -= listener;
	return SapphireStorageError::SUCCESS;
}

SapphireStorageError LocalSapphireDataStorage::addMessagesChangedListener(MessagesChangedListener& listener) {
	MutexLocker lock { messagesChangedListenersMutex };
	messagesChangedEvents += listener;
	return SapphireStorageError::SUCCESS;
}
SapphireStorageError LocalSapphireDataStorage::removeMessagesChangedListener(MessagesChangedListener& listener) {
	MutexLocker lock { messagesChangedListenersMutex };
	messagesChangedEvents -= listener;
	return SapphireStorageError::SUCCESS;
}

SapphireStorageError LocalSapphireDataStorage::addHardwareProgressChangedListener(const SapphireUUID& hardware,
		HardwareProgressChangedListener& listener) {
	auto* h = getHardwareCreate(hardware);
	MutexLocker ml = hardwaresLockPool.locker(h->hardwareUUID);
	h->hardwareProgressChangedEvents += listener;
	return SapphireStorageError::SUCCESS;
}

SapphireStorageError LocalSapphireDataStorage::removeHardwareProgressChangedListener(const SapphireUUID& hardware,
		HardwareProgressChangedListener& listener) {
	auto* h = findHardware(hardware);
	if (h == nullptr) {
		return SapphireStorageError::INVALID_HARDWARE;
	}
	MutexLocker ml = hardwaresLockPool.locker(h->hardwareUUID);
	h->hardwareProgressChangedEvents -= listener;
	return SapphireStorageError::SUCCESS;
}
SapphireStorageError LocalSapphireDataStorage::addHardwareAssociationListener(HardwareAssociationListener& listener) {
	MutexLocker ml { hardwareMutex };
	hardwareAssociationEvents += listener;
	return SapphireStorageError::SUCCESS;
}

SapphireStorageError LocalSapphireDataStorage::removeHardwareAssociationListener(HardwareAssociationListener& listener) {
	MutexLocker ml { hardwareMutex };
	hardwareAssociationEvents -= listener;
	return SapphireStorageError::SUCCESS;
}

SapphireStorageError LocalSapphireDataStorage::loginUser(const SapphireUUID& uuid, RegistrationToken* outtoken, FixedString* outname,
		SapphireDifficulty* outdiffcolor) {
	if (!uuid) {
		return SapphireStorageError::INVALID_USER_UUID;
	}
	auto* u = findUser(uuid);
	if (u == nullptr) {
		return SapphireStorageError::USER_NOT_FOUND;
	}
	MutexLocker ml = usersLockPool.locker(u->uuid);
	*outtoken = u->registrationToken;
	*outname = u->name;
	*outdiffcolor = u->difficultyColor;
	return SapphireStorageError::SUCCESS;
}
SapphireStorageError LocalSapphireDataStorage::registerUser(const SapphireUUID& uuid, const RegistrationToken& token) {
	if (!uuid) {
		return SapphireStorageError::INVALID_USER_UUID;
	}
	MutexLocker l { usersMutex };
	auto* found = findUserLocked(uuid);
	if (found != nullptr) {
		return SapphireStorageError::USER_ALREADY_REGISTERED;
	}

	StorageSapphireUser* u = new StorageSapphireUser(uuid);
	u->registrationToken = token;
	StorageDirectoryDescriptor userdir { usersDirectory.getPath() + (const char*) u->getUUID().asString() };
	userdir.create();
	saveUser(userdir, *u);
	users.setSorted(u, StorageSapphireUser::compare);

	return SapphireStorageError::SUCCESS;
}
SapphireStorageError LocalSapphireDataStorage::updateUserInfo(const SapphireUUID& uuid, const FixedString& name,
		SapphireDifficulty diffcolor) {
	auto* user = findUser(uuid);
	if (user == nullptr) {
		return SapphireStorageError::USER_NOT_FOUND;
	}
	MutexLocker ml = usersLockPool.locker(user->uuid);
	return updateUserInfoLocked(user, name, diffcolor);
}
SapphireStorageError LocalSapphireDataStorage::updateUserInfoLocked(StorageSapphireUser* user, const FixedString& name,
		SapphireDifficulty diffcolor) {
	if (user->name == name && user->difficultyColor == diffcolor) {
		return SapphireStorageError::SUCCESS;
	}
	user->name = name;
	user->difficultyColor = diffcolor;
	StorageDirectoryDescriptor userdir { usersDirectory.getPath() + (const char*) user->getUUID().asString() };
	userdir.create();
	saveUser(userdir, *user);
	return SapphireStorageError::SUCCESS;
}

SapphireStorageError LocalSapphireDataStorage::setLevelProgress(ProgressSynchId* progressid, const SapphireUUID& hardware,
		const SapphireUUID& level, SapphireLevelProgress progress) {
	//do not check existence of level, as it might not be uploaded yet

	auto* h = getHardwareCreate(hardware);

	MutexLocker ml = hardwaresLockPool.locker(h->hardwareUUID);
	if (h->progressId != *progressid) {
		*progressid = h->progressId;
		return SapphireStorageError::INVALID_PROGRESSID;
	}
	LOGTRACE()<< "Set level progress: " << *progressid << " uuid: " << level.asString();
	h->progressId++;
	//always append to the progress file
	StorageFileDescriptor fd { hardwareDirectory.getPath() + (const char*) hardware.asString() + FILENAME_HARDWARE_PROGRESS };
	{
		auto&& ostream = EndianOutputStream<Endianness::Big>::wrap(fd.openAppendStream());
		ostream.serialize<SapphireUUID>(level);
		ostream.serialize<uint32>((uint32) progress);
	}
	broadcastListenerEventsNoMutex(h->hardwareProgressChangedEvents, *progressid, level, progress);
	if (h->isFinishedLevel(level)) {
		return SapphireStorageError::PROGRESS_UNCHANGED;
	}
	if (progress == SapphireLevelProgress::LEVEL_SEEN) {
		if (h->isSeenLevel(level)) {
			return SapphireStorageError::PROGRESS_UNCHANGED;
		}
		h->seenLevels.setSorted(new SapphireUUID(level), compareUUIDPtrs);
	} else {
		h->finishedLevels.setSorted(new SapphireUUID(level), compareUUIDPtrs);
	}

	return SapphireStorageError::SUCCESS;
}
SapphireStorageError LocalSapphireDataStorage::getProgressId(const SapphireUUID& hardware, ProgressSynchId* outprogressid) {
	auto* h = getHardwareCreate(hardware);
	if (h == nullptr) {
		*outprogressid = 0;
		return SapphireStorageError::SUCCESS;
	}
	MutexLocker ml = hardwaresLockPool.locker(h->hardwareUUID);
	*outprogressid = h->progressId;
	return SapphireStorageError::SUCCESS;
}

SapphireStorageError LocalSapphireDataStorage::getAssociatedHardwareProgressId(const SapphireUUID& hardware,
		const SapphireUUID& associatedhardware, ProgressSynchId* outprogressid) {
	auto* h = getHardwareCreate(hardware);
	if (h == nullptr) {
		return SapphireStorageError::INVALID_HARDWARE;
	}
	MutexLocker ml = hardwaresLockPool.locker(h->hardwareUUID);
	auto* associated = h->getAssociatedHardware(associatedhardware);
	if (associated == nullptr) {
		return SapphireStorageError::HARDWARE_NOT_ASSOCIATED;
	}
	*outprogressid = associated->synchronizedProgressCount;
	return SapphireStorageError::SUCCESS;
}
SapphireStorageError LocalSapphireDataStorage::increaseAssociatedHardwareProgressId(const SapphireUUID& hardware,
		const SapphireUUID& associatedhardware, ProgressSynchId* progressid) {
	auto* h = getHardwareCreate(hardware);
	if (h == nullptr) {
		return SapphireStorageError::INVALID_HARDWARE;
	}
	MutexLocker ml = hardwaresLockPool.locker(h->hardwareUUID);
	auto* associated = h->getAssociatedHardware(associatedhardware);
	if (associated == nullptr) {
		return SapphireStorageError::HARDWARE_NOT_ASSOCIATED;
	}
	if (associated->synchronizedProgressCount != *progressid) {
		*progressid = associated->synchronizedProgressCount;
		return SapphireStorageError::INVALID_PROGRESSID;
	}
	associated->synchronizedProgressCount = *progressid + 1;
	h->saveAssociatedHardwares(hardwareDirectory.getPath() + (const char*) hardware.asString());
	return SapphireStorageError::SUCCESS;
}

void LocalSapphireDataStorage::StorageUserHardware::loadProgress(const FilePath& directory) {
	StorageFileDescriptor progfd { directory + FILENAME_HARDWARE_PROGRESS };
	loadProgress(progfd);

	StorageFileDescriptor associated { directory + FILENAME_ASSOCIATED_HARDWARES };
	auto&& ais = EndianInputStream<Endianness::Big>::wrap(associated.openInputStream());
	while (true) {
		SapphireUUID hardware;
		ProgressSynchId progress;
		if (!ais.deserialize<SapphireUUID>(hardware) || !ais.deserialize<ProgressSynchId>(progress)) {
			break;
		}
		associatedHardwares.setSorted(new AssociatedHardware(hardware, progress), AssociatedHardware::compare);
	}

}
void LocalSapphireDataStorage::StorageUserHardware::saveAssociatedHardwares(const FilePath& directory) {
	StorageFileDescriptor associated { directory + FILENAME_ASSOCIATED_HARDWARES };
	auto&& os = EndianOutputStream<Endianness::Big>::wrap(associated.openOutputStream());
	for (auto&& h : associatedHardwares) {
		os.serialize<SapphireUUID>(h->hardwareUUID);
		os.serialize<ProgressSynchId>(h->synchronizedProgressCount);
	}
}
void LocalSapphireDataStorage::StorageUserHardware::loadProgress(FileDescriptor& fd) {
	auto&& istream = EndianInputStream<Endianness::Big>::wrap(fd.openInputStream());
	SapphireUUID uuid;
	SapphireLevelProgress progress;
	long long pos = 0;
	while (true) {
		pos = istream.getPosition();
		if (!istream.deserialize<SapphireUUID>(uuid) || !istream.deserialize<uint32>(reinterpret_cast<uint32&>(progress))) {
			if (istream.getPosition() != pos) {
				postLogEvent(FixedString { "Failed to deserialize hardware progress data." });
			}
			break;
		}
		if (progress == SapphireLevelProgress::LEVEL_FINISHED) {
			finishedLevels.setSorted(new SapphireUUID(uuid), compareUUIDPtrs);
		} else {
			seenLevels.setSorted(new SapphireUUID(uuid), compareUUIDPtrs);
		}
		++progressId;
	}
}

void LocalSapphireDataStorage::StorageSapphireUser::loadUploadedLevels(const FilePath& directory) {
	StorageFileDescriptor fd { directory + FILENAME_USER_UPLOADED_LEVELS };
	auto&& istream = EndianInputStream<Endianness::Big>::wrap(fd.openInputStream());

	for (SapphireUUID uuid; istream.deserialize<SapphireUUID>(uuid);) {
		uploadedLevels.setSorted(new SapphireUUID(uuid), compareUUIDPtrs);
	}
}

void LocalSapphireDataStorage::StorageSapphireUser::saveUploadedLevels(const FilePath& directory) {
	StorageFileDescriptor fd { directory + FILENAME_USER_UPLOADED_LEVELS };
	auto&& ostream = EndianOutputStream<Endianness::Big>::wrap(fd.openOutputStream());

	for (auto&& l : uploadedLevels) {
		ostream.serialize<SapphireUUID>(*l);
	}
}

SapphireStorageError LocalSapphireDataStorage::getLevelStatistics(const SapphireUUID& leveluuid, LevelStatistics* outstats,
		unsigned int* outplaycount) {
	auto* stats = findStatistics(leveluuid);
	if (stats == nullptr) {
		return SapphireStorageError::STATS_NOT_FOUND;
	}
	MutexLocker ml = statisticsLevelLockPool.locker(stats->levelUUID);
	*outstats = stats->stats;
	*outplaycount = stats->playCount;
	return SapphireStorageError::SUCCESS;
}

void LocalSapphireDataStorage::applyLeaderboardData(StorageSapphireUser* user, StorageLevelStatistics* foundstats,
		const LevelStatistics& stats, PlayerDemoId demoid, unsigned int demotime) {
	foundstats->getLeaderboard(SapphireLeaderboards::MostGems).addEntry(
			new StorageLeaderboardEntry(user, stats.getCollectedGemWorth(), demoid),
			StorageLeaderboardEntry::comparatorMostGems);
	foundstats->getLeaderboard(SapphireLeaderboards::LeastSteps).addEntry(
			new StorageLeaderboardEntry(user, stats.moveCount, demoid),
			StorageLeaderboardEntry::comparatorLeastSteps);
	foundstats->getLeaderboard(SapphireLeaderboards::LeastTime).addEntry(
			new StorageLeaderboardEntry(user, demotime, demoid),
			StorageLeaderboardEntry::comparatorLeastTime);
}
LocalSapphireDataStorage::StorageLevelStatistics* LocalSapphireDataStorage::findStatistics(const SapphireUUID& leveluuid) {
	MutexLocker l { statisticsMutex };
	return findStatisticsLocked(leveluuid);
}
LocalSapphireDataStorage::StorageLevelStatistics* LocalSapphireDataStorage::findStatisticsLocked(const SapphireUUID& leveluuid) {
	int index = statistics.getIndexForSorted(leveluuid, StorageLevelStatistics::compareStatsUUID);
	if (index < 0) {
		return nullptr;
	}
	return statistics.get(index);
}

SapphireStorageError LocalSapphireDataStorage::appendLevelStatistics(const SapphireUUID& leveluuid, const SapphireUUID& userid,
		const FixedString& steps, uint32 randomseed) {
	if (steps.length() == 0) {
		return SapphireStorageError::LEVEL_NOT_SUCCESSFULLY_FINISHED;
	}
	auto* user = findUser(userid);
	if (user == nullptr) {
		return SapphireStorageError::USER_NOT_FOUND;
	}
	Level level;
	bool levelbuiltin;
	auto levelerror = getLevel(leveluuid, &level, &levelbuiltin);
	if (levelerror != SapphireStorageError::SUCCESS) {
		return levelerror;
	}
	level.setRandomSeed(randomseed);
	DemoPlayer::playMovesUntilSuccess(steps, steps.length() / level.getPlayerCount(), level);
	if (!level.isSuccessfullyOver()) {
		return SapphireStorageError::LEVEL_NOT_SUCCESSFULLY_FINISHED;
	}
	LOGTRACE()<< "Appending level statistics: " << level.getInfo().title;
	auto&& stats = level.getStatistics();

	StorageLevelStatistics* foundstats;
	{
		MutexLocker ml { statisticsMutex };
		foundstats = findStatisticsLocked(leveluuid);
		if (foundstats == nullptr) {
			foundstats = new StorageLevelStatistics(leveluuid);
			statistics.setSorted(foundstats, StorageLevelStatistics::compareStats);
		}
	}
	MutexLocker ml = statisticsLevelLockPool.locker(foundstats->levelUUID);
	foundstats->addStats(stats);

	PlayerDemoId demoid = foundstats->demoId++;
	{
		StorageFileDescriptor demofd { demosDirectory.getPath() + (const char*) leveluuid.asString() };
		if (demoid % StorageLevelStatistics::DEMO_OFFSET_PARTITION == 0) {
			auto size = demofd.size();
			//might not exist yet
			ASSERT((size < 0 && demoid == 0) || size >= 0);
			foundstats->playerDemoOffsets.add(new uint64(size < 0 ? 0 : size));
		}

		auto&& demoostream = EndianOutputStream<Endianness::Big>::wrap(demofd.openAppendStream());
		demoostream.serialize<SapphireUUID>(userid);
		demoostream.serialize<uint32>(randomseed);
		demoostream.serialize<FixedString>(steps);
	}
	applyLeaderboardData(user, foundstats, stats, demoid, level.getTurn());

	return SapphireStorageError::SUCCESS;
}

SapphireStorageError LocalSapphireDataStorage::getPlayerDemo(const SapphireUUID& leveluuid, PlayerDemoId demoid, FixedString* outsteps,
		uint32* outrandomseed) {
	StorageFileDescriptor demofd { demosDirectory.getPath() + (const char*) leveluuid.asString() };

	auto* stats = findStatistics(leveluuid);
	if (stats == nullptr || demoid >= stats->demoId) {
		return SapphireStorageError::DEMO_NOT_FOUND;
	}

	SapphireUUID userid;
	uint32 randomseed;
	PlayerDemoId currentdemoid = 0;

	LOGI()<< "Get player demo at " << demoid << " for " << leveluuid.asString();

	MutexLocker ml = statisticsLevelLockPool.locker(leveluuid);
	auto&& istream = EndianInputStream<Endianness::Big>::wrap(demofd.openInputStream());
	if (demoid / StorageLevelStatistics::DEMO_OFFSET_PARTITION < stats->playerDemoOffsets.size()) {
		uint64* offset = stats->playerDemoOffsets.get(demoid / StorageLevelStatistics::DEMO_OFFSET_PARTITION);
		LOGI()<< "Skip demo file to " << *offset;
		istream.skip(*offset);
		currentdemoid = demoid - (demoid % StorageLevelStatistics::DEMO_OFFSET_PARTITION);
	}

	while (currentdemoid <= demoid) {
		if (currentdemoid == demoid) {
			if (!istream.deserialize<SapphireUUID>(userid) || !istream.deserialize<uint32>(*outrandomseed)
					|| !istream.deserialize<SafeFixedString<SAPPHIRE_DEMO_MAX_LEN>>(*outsteps)) {
				return SapphireStorageError::STORAGE_UNAVAILABLE;
			}
			return SapphireStorageError::SUCCESS;
		} else {
			if (!istream.deserialize<SapphireUUID>(userid) || !istream.deserialize<uint32>(randomseed)
					|| !istream.deserialize<IgnoreFixedString>(nullptr)) {
				return SapphireStorageError::DEMO_NOT_FOUND;
			}
		}
		++currentdemoid;
	}

	return SapphireStorageError::STORAGE_UNAVAILABLE;
}

SapphireStorageError LocalSapphireDataStorage::getLeaderboard(const SapphireUUID& leveluuid, const SapphireUUID& userid,
		SapphireLeaderboards leaderboardtype, unsigned int maxoutcount, ArrayList<LeaderboardEntry>* outentries, int* outuserindex,
		uint32* outuserscore, int32* outuserposition, PlayerDemoId* outuserdemoid, uint32* outtotalcount) {
	auto* foundstats = findStatistics(leveluuid);
	if (foundstats == nullptr) {
		return SapphireStorageError::LEADERBOARD_NOT_FOUND;
	}
	MutexLocker ml = statisticsLevelLockPool.locker(foundstats->levelUUID);
	const auto* lb = foundstats->getLeaderboardIfExists(leaderboardtype);
	if (lb == nullptr) {
		return SapphireStorageError::LEADERBOARD_NOT_FOUND;
	}
	unsigned int count = lb->entries.size();
	unsigned int position = 0;
	uint64 prevscore = 0xffffffffffffffff;

	*outuserindex = -1;
	*outtotalcount = count;
	for (unsigned int i = 0; i < count; ++i) {
		auto&& l = lb->entries[i];
		if (*outuserindex < 0) {
			if (l.score != prevscore) {
				++position;
				prevscore = l.score;
			}
			if (l.user->uuid == userid) {
				*outuserindex = i;
				*outuserscore = l.score;
				*outuserposition = position;
				*outuserdemoid = l.demoId;
			}
		}
		if (i < maxoutcount) {
			LeaderboardEntry* le = new LeaderboardEntry();
			le->demoId = l.demoId;
			le->score = l.score;
			le->userName = l.user->name;
			outentries->add(le);
		} else {
			if (*outuserindex >= 0) {
				//we are not interested in entries after the user, and after max out count
				break;
			}
		}
	}
	return SapphireStorageError::SUCCESS;
}

} // namespace userapp

