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
 * CommunityConnection.h
 *
 *  Created on: 2016. okt. 16.
 *      Author: sipka
 */

#ifndef TEST_SAPPHIRE_COMMUNITY_COMMUNITYCONNECTION_H_
#define TEST_SAPPHIRE_COMMUNITY_COMMUNITYCONNECTION_H_

#include <framework/utils/LinkedList.h>
#include <framework/utils/FixedString.h>
#include <framework/utils/BasicListener.h>
#include <framework/utils/ArrayList.h>
#include <framework/io/stream/LoopedInputStream.h>
#include <framework/io/stream/OutputStream.h>
#include <framework/io/stream/InputStream.h>
#include <framework/io/files/StorageFileDescriptor.h>
#include <framework/io/files/StorageDirectoryDescriptor.h>
#include <framework/threading/Mutex.h>
#include <framework/threading/Semaphore.h>
#include <framework/random/Randomer.h>
#include <sapphire/level/SapphireLevelDescriptor.h>
#include <sapphire/common/commontypes.h>

#include <sapphire/community/SapphireUser.h>
#include <sapphire/AsynchronTask.h>
#include <util/RC4Cipher.h>
#include <util/RC4Stream.h>

#include <sapphire/community/SapphireDiscussionMessage.h>
#include <sapphire/server/WorkerThread.h>

#include <framework/io/network/ipv4/tcp/TCPIPv4Connection.h>

namespace userapp {
using namespace rhfw;

class SapphireUILayer;
class Level;
class SapphireScene;
class SapphireLevelDetails;
class RegistrationToken;
class CommunityConnection {
public:
	class StateListener: public BasicListener<StateListener> {
	public:
		virtual void onConnected(CommunityConnection* connection) {
		}
		virtual void onLoggedIn(CommunityConnection* connection) {
		}
		virtual void onDisconnected(CommunityConnection* connection) {
		}
		virtual void onConnectionFailed(CommunityConnection* connection) {
		}
	};
	class OnlineUser {
		SapphireUUID connectionId;
		FixedString userName;
		SapphireDifficulty difficultyLevel;
	public:
		OnlineUser(const SapphireUUID& connectionid, FixedString username, SapphireDifficulty difflevel)
				: connectionId(connectionid), userName(util::move(username)), difficultyLevel(difflevel) {
		}

		const SapphireUUID& getConnectionId() const {
			return connectionId;
		}
		const FixedString& getUserName() const {
			return userName;
		}
		SapphireDifficulty getDifficultyLevel() const {
			return difficultyLevel;
		}
		FixedString& getUserName() {
			return userName;
		}
		SapphireDifficulty& getDifficultyLevel() {
			return difficultyLevel;
		}
	};
	class LeaderboardEntry {
	public:
		uint32 score;
		FixedString userName;
		PlayerDemoId demoId;
	};

	using LevelUploadListener = SimpleListener<void(SapphireCommError, const SapphireUUID&)>;
	using LevelDownloadListener = SimpleListener<void(const SapphireUUID&, SapphireCommError, const Level*)>;
	using LevelsQueriedListener = SimpleListener<void(unsigned int index, const SapphireLevelDetails*)>;
	using LevelRemovedListener = SimpleListener<void(const SapphireLevelDetails*)>;
	using CommunityInformationListener = SimpleListener<void(SapphireCommunityInformation)>;
	/**
	 * param:
	 * uint32: user specified message id
	 * bool: success
	 */
	using SendMessageListener = SimpleListener<void(uint32, bool)>;
	using DiscussionMessagesChangedListener = SimpleListener<void(unsigned int, unsigned int)>;
	/**
	 * param:
	 * unsigned int: index
	 */
	using DiscussionMessageArrivedListener = SimpleListener<void(unsigned int)>;
	/**
	 * param:
	 * OnlineUser: user
	 * bool: true if online
	 */
	using UserStateChangedListener = SimpleListener<void(const OnlineUser&, bool)>;

	/**
	 * param:
	 * uint32: the identifier
	 */
	using LinkIdentifierRequestListener = SimpleListener<void(uint32)>;

	/**
	 * param:
	 * SapphireCommError: the error occurred during linking. NoError means success
	 */
	using LinkResultListener = SimpleListener<void(SapphireCommError)>;

	/**
	 * param:
	 * SapphireCommError: error
	 * SapphireUUID: level uuid
	 * LevelStatistics: the statistics
	 * unsigned int: the play count
	 */
	using StatisticsDownloadListener = SimpleListener<void(SapphireCommError error, const SapphireUUID& leveluuid, const LevelStatistics* stats, unsigned int playcount)>;

	using PlayerDemoDownloadedListener = SimpleListener<void(const SapphireUUID& leveluuid, PlayerDemoId demoid, SapphireCommError error, const FixedString& steps, uint32 randomseed)>;

	using LeaderboardDownloadedListener = SimpleListener<void(const SapphireUUID& leveluuid, SapphireLeaderboards type, SapphireCommError error,
			const ArrayList<LeaderboardEntry>* entries, int32 userindex, uint32 userscore, int32 userposition, PlayerDemoId userdemoid, uint32 totalcount)>;
private:
	TCPIPv4Connection connection;

	AsynchronTask readTask;
	bool connectionTaskRunning = false;
	WorkerThread writeWorker;

	RC4Cipher writeCipher;
	EndianOutputStream<Endianness::Big>* normalOutputStream = nullptr;
	EndianOutputStream<Endianness::Big>* cipherOutputStream = nullptr;
	EndianOutputStream<Endianness::Big>* outputStream = nullptr;

	SapphireUUID loggedHardwareUUID;
	bool loggedIn = false;

	Randomer* randomer = nullptr;

	ArrayList<SapphireLevelDetails> levelDetails;
	unsigned int messagesRemoteStartIndex = 0;
	unsigned int messagesRemoteCount = 0;
	unsigned int messagesLocalStartIndex = 0;
	ArrayList<SapphireDiscussionMessage> messages;
	LinkedList<OnlineUser> onlineUsers;

	StateListener::Events stateEvents;

	SapphireUUID userUUID;

	SapphireScene* scene;

	ProgressSynchId serverProgressSynchId = 0;

	uint32 linkIdentifier = 0;

	void readThreadFunction(AsynchronTask& task, const SapphireUUID& userid);

	void postUpgradeStream();
	void postLogin();
	void postUpdatePlayerData(const FixedString& name, SapphireDifficulty diffcolor);

	void postUploadLevel(const Level& level);

	bool saveRegistrationToken(const RegistrationToken& regtoken);

	void postPingRequest(uint32 extra);
	void postPingResponse(uint32 extra);

	void postLevelProgress(ProgressSynchId progressid, const SapphireUUID& leveluuid, const FixedString& steps, uint32 randomseed,
			SapphireLevelCommProgress progress);

public:
	LevelUploadListener::Events levelUploadEvents;
	LevelDownloadListener::Events levelDownloadEvents;
	LevelsQueriedListener::Events levelsQueriedEvents;
	LevelRemovedListener::Events levelRemovedEvents;
	SendMessageListener::Events sendMessageEvents;
	DiscussionMessagesChangedListener::Events discussionMessagesChangedEvents;
	CommunityInformationListener::Events communityInformationEvents;
	DiscussionMessageArrivedListener::Events discussionMessageArrivedEvents;
	UserStateChangedListener::Events userStateChangedEvents;
	LinkIdentifierRequestListener::Events linkIdentifierRequestEvents;
	LinkResultListener::Events linkResultEvents;
	PlayerDemoDownloadedListener::Events playerDemoDownloadedEvents;
	LeaderboardDownloadedListener::Events leaderboardDownloadedEvents;

	StatisticsDownloadListener::Events statisticsDownloadEvents;

	explicit CommunityConnection(SapphireScene* scene);
	~CommunityConnection();

	void addStateListenerAndConnect(SapphireScene* scene, LinkedNode<StateListener>& listener);
	void addStateListener(LinkedNode<StateListener>& listener);

	void connect(SapphireScene* scene);

	void uploadLevel(const Level& level);

	void requestLevels();

	void downloadLevel(const SapphireLevelDetails* details);
	void downloadLevel(const SapphireUUID& leveluuid);

	void updatePlayerData(const SapphireUser& user, SapphireDifficulty diffcolor) {
		updatePlayerData(user.getUserName(), diffcolor);
	}
	void updatePlayerData(const FixedString& username, SapphireDifficulty diffcolor) {
		postUpdatePlayerData(username, diffcolor);
	}

	bool isConnectionEstablished() const {
		return connectionTaskRunning && loggedHardwareUUID;
	}
	bool isLoggedIn() const {
		return loggedIn;
	}
	bool isTaskRunning() const {
		return connectionTaskRunning;
	}

	ArrayList<SapphireLevelDetails>& getLevelDetails() {
		return levelDetails;
	}

	void rateLevel(const SapphireUUID& leveluuid, unsigned int rating);

	void sendMessage(const char* message, uint32 userid);

	void sendLinkIdentifier(uint32 identifier);

	bool reportLevel(const SapphireUUID& leveluuid, const FixedString& reason);

	void requestCommunityNotifications(bool needs);

	LinkedList<OnlineUser>& getOnlineUsers() {
		return onlineUsers;
	}

	ArrayList<SapphireDiscussionMessage>& getMessages() {
		return messages;
	}

	void requestMessages(unsigned int start, unsigned int count);
	void queryMessageRanges() {
		requestMessages(0, 0);
	}

	unsigned int getMessagesLocalStartIndex() const {
		return messagesLocalStartIndex;
	}

	unsigned int getMessagesRemoteStartIndex() const {
		return messagesRemoteStartIndex;
	}
	unsigned int getMessagesRemoteCount() const {
		return messagesRemoteCount;
	}

	bool showCommunityInformationDialog(SapphireCommunityInformation info, SapphireUILayer* parent);

	void notifyLevelProgress(ProgressSynchId progressid, const SapphireUUID& leveluuid, const FixedString& steps, uint32 randomseed,
			SapphireLevelCommProgress progress);

	void sendLinkIdentifierRequest();
	void sendLinkCancel();
	void sendLinkRemoteIdentifier(uint32 identifier);
	void sendGetStatistics(const SapphireUUID& level);

	void sendGetLeaderboards(const SapphireUUID& level, SapphireLeaderboards leaderboard);
	void sendGetDemo(const SapphireUUID& leveluuid, PlayerDemoId demoid);

	void checkPostLevelProgress();

	TCPIPv4Address getCommunityServerAddress() const;
};

} // namespace userapp

#endif /* TEST_SAPPHIRE_COMMUNITY_COMMUNITYCONNECTION_H_ */
