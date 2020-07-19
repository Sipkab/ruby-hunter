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
 * SapphireScene.h
 *
 *  Created on: 2016. apr. 23.
 *      Author: sipka
 */

#ifndef TEST_SAPPHIRE_SAPPHIRESCENE_H_
#define TEST_SAPPHIRE_SAPPHIRESCENE_H_

#include <framework/threading/Semaphore.h>
#include <framework/scene/Scene.h>
#include <framework/utils/ArrayList.h>
#include <framework/utils/FixedString.h>
#include <framework/utils/BasicListener.h>
#include <framework/io/files/StorageFileDescriptor.h>
#include <framework/io/files/StorageDirectoryDescriptor.h>
#include <framework/resource/Resource.h>
#include <framework/random/RandomContext.h>
#include <framework/utils/LinkedNode.h>
#include <framework/utils/LinkedList.h>
#include <framework/io/gamepad/GamePadState.h>

#include <sapphire/common/commontypes.h>
#include <appmain.h>
#include <sapphire/level/SapphireRandom.h>
#include <sapphire/level/Demo.h>
#include <sapphire/level/Level.h>
#include <sapphire/level/SapphireUUID.h>
#include <sapphire/dialogs/DialogLayer.h>
#include <sapphire/community/SapphireUser.h>
#include <sapphire/community/CommunityConnection.h>
#include <sapphire/level/SapphireLevelDescriptor.h>
#include <sapphire/common/RegistrationToken.h>
#include <sapphire/dialogs/items/BusyIndicatorDialogItem.h>
#include <sapphire/util/GamePadKeyRepeater.h>
#include <sapphire/SapphireSteamAchievement.h>
#include <sapphire/steam_opt.h>

#include <gen/assets.h>
#include <gen/types.h>
#include <gen/platform.h>

namespace userapp {
using namespace rhfw;

class SapphireUILayer;
class SapphireScene;
class AsynchronTask;
class SapphireBackgroundLayer;
class LevelStatisticsDialog;

enum class GameScale {
	SMALLEST = 0,
	SMALL = 1,
	NORMAL = 2,
	BIG = 3,
	BIGGEST = 4,
	_count_of_entries = 5,
};

enum class SapphireFullScreenState {
	EXCLUSIVE_FULLSCREEN = 0,
	BORDERLESS_FULLSCREEN = 1,
	WINDOWED = 2,
	_count_of_entries = 3,
};
enum class SapphireArtStyle
	: uint32 {
		RETRO_2D = 0,
	PERSPECTIVE_3D = 1,
	ORTHO_3D = 2,

	FLAG_3D = 1 | 2,

	MIN = 0,
	MAX = 2,
	UNUSED = MAX + 1,

	_count_of_entries = 3,
};
enum class SapphireKeyCode {
	KEY_P1_UP = 0,
	KEY_P1_RIGHT,
	KEY_P1_DOWN,
	KEY_P1_LEFT,
	KEY_P1_BOMB,
	KEY_P1_PICK,
	KEY_P2_UP,
	KEY_P2_RIGHT,
	KEY_P2_DOWN,
	KEY_P2_LEFT,
	KEY_P2_BOMB,
	KEY_P2_PICK,
	//end of v1

	KEY_RESET_LEVEL,
	KEY_EDITOR_PAINT,
	KEY_QUICK_SUSPEND,

	KEY_INCREASE_SPEED,
	KEY_DECREASE_SPEED,

	_count_of_entries = 12,
	_count_of_entries_v2 = 15,
	_count_of_entries_v3 = 17,
	_count_of_entries_vLatest = _count_of_entries_v3,
};
class SapphireKeyMap {
private:
	void fixControlCommandKeys();
public:
	class gamepad_init {
	};
	static const unsigned int KEY_COUNT = (unsigned int) SapphireKeyCode::_count_of_entries_vLatest;
	KeyCode keyCodeMap[KEY_COUNT];

	SapphireKeyMap();
	SapphireKeyMap(const gamepad_init& marker);

	KeyCode operator[](SapphireKeyCode kc) const {
		return keyCodeMap[(unsigned int) kc];
	}
	void set(SapphireKeyCode skc, KeyCode kc) {
		this->keyCodeMap[(unsigned int) skc] = kc;
	}
	template<typename OStream>
	bool serialize(OStream&& os) {
		for (unsigned int i = 0; i < KEY_COUNT; ++i) {
			bool r = os.template serialize<uint32>((uint32) keyCodeMap[i]);
			if (!r) {
				return false;
			}
		}
		return true;
	}
	template<typename IStream>
	bool deserialize(unsigned int version, IStream&& is) {
		unsigned int len;
		if (version <= 4) {
			len = (unsigned int) SapphireKeyCode::_count_of_entries;
		} else if (version <= 5) {
			len = (unsigned int) SapphireKeyCode::_count_of_entries_v2;
		} else {
			len = KEY_COUNT;
		}
		for (unsigned int i = 0; i < len; ++i) {
			bool r = is.template deserialize<uint32>(reinterpret_cast<uint32&>(keyCodeMap[i]));
			if (!r) {
				return false;
			}
		}
		return true;
	}
};
class SapphireSettings {
	friend class SapphireScene;
private:
public:
	uint32 music = 100;
	uint32 sound = 100;
	SapphireArtStyle artStyle = SapphireArtStyle::PERSPECTIVE_3D;

	GameScale gameScale = GameScale::NORMAL;
	GameScale uiScale = GameScale::NORMAL;

	SapphireFullScreenState fullScreenState = SapphireFullScreenState::BORDERLESS_FULLSCREEN;

	bool vsync = true;
	unsigned int multiSampleFactor = 0;

	SapphireUUID hardwareUUID;

	FixedString openingMusicName = nullptr;

	uint32 soundYamYam = 60;
	uint32 soundLaser = 60;
	uint32 soundExplosion = 40;
	uint32 soundConvert = 60;
	uint32 soundPickUp = 80;
	uint32 soundDoorUsing = 60;
	uint32 soundFalling = 60;
	uint32 soundEnemies = 60;
	uint32 soundWheel = 80;
	uint32 soundGems = 60;

	bool leftHandedOnScreenControls = false;
	bool steamAchievementProgressIndicatorEnabled = true;

	SapphireArtStyle getArtStyleForLevel(SapphireDifficulty difficulty) const {
		return artStyle;
	}

	bool is3DArtStyle() const {
		return artStyle != SapphireArtStyle::RETRO_2D;
	}
};
LevelState sapphireCommProgressToState(SapphireLevelCommProgress progress);
SapphireLevelCommProgress levelStateToSapphireCommProgress(LevelState state);
class Refreshable;
class LeaderboardsDialog;
class SapphireSteamAchievement;
class SapphireScene: public rhfw::Scene,
		private audio::SoundPlayerToken::StoppedListener,
		private core::WindowAccesStateListener,
		private CommunityConnection::StateListener,
		private GamePadStateListener,
		private core::TimeListener {
private:
	using LoginConnectionTask = SimpleListener<void()>;
	LoginConnectionTask::Events loginConnectionTasks;

	bool needBackground = true;
	ArrayList<SapphireLevelDescriptor> levels[2][(unsigned int) SapphireDifficulty::_count_of_entries];
	unsigned int finishedStatistics[2][(unsigned int) SapphireDifficulty::_count_of_entries] { { 0 } };
	unsigned int seenStatistics[2][(unsigned int) SapphireDifficulty::_count_of_entries] { { 0 } };
	class UnknownLevelState {
	public:
		SapphireUUID levelUUID;
		LevelState state;
	};
	ArrayList<UnknownLevelState> unknownLevelStates;

	SapphireSettings settings;
	SapphireKeyMap keyMap;
	SapphireKeyMap gamepadKeyMap;

	StorageFileDescriptor settingsFile { StorageDirectoryDescriptor::Root() + "b" };

	StorageDirectoryDescriptor dataDirectory { StorageDirectoryDescriptor::Root() + "data" };
	StorageDirectoryDescriptor musicDirectory { dataDirectory.getPath() + SAPPHIRE_MUSIC_DIRECTORY_NAME };
	StorageFileDescriptor levelStatesFile { dataDirectory.getPath() + "a" };
	StorageFileDescriptor progressFile { dataDirectory.getPath() + "prg" };
	StorageFileDescriptor statisticsFile { dataDirectory.getPath() + "stats" };
	StorageFileDescriptor userUUIDFile { dataDirectory.getPath() + "c" };
	StorageFileDescriptor registrationTokenFile { dataDirectory.getPath() + "r" };
	StorageDirectoryDescriptor levelDemosDirectory { dataDirectory.getPath() + "d" };
	StorageDirectoryDescriptor userLevelsDirectory { dataDirectory.getPath() + "levels" };
	StorageDirectoryDescriptor downloadsDirectory { dataDirectory.getPath() + "dl" };
	StorageDirectoryDescriptor suspendedDirectory { dataDirectory.getPath() + "suspend" };

	/**
	 * Pre 5 version files
	 */
	StorageFileDescriptor levelStatesFile_old { StorageDirectoryDescriptor::Root() + "a" };
	StorageFileDescriptor registrationTokenFile_old { StorageDirectoryDescriptor::Root() + "r" };
	StorageDirectoryDescriptor levelDemosDirectory_old { StorageDirectoryDescriptor::Root() + "d" };
	StorageDirectoryDescriptor userLevelsDirectory_old { StorageDirectoryDescriptor::Root() + "levels" };
	StorageDirectoryDescriptor downloadsDirectory_old { StorageDirectoryDescriptor::Root() + "dl" };
	StorageDirectoryDescriptor suspendedDirectory_old { StorageDirectoryDescriptor::Root() + "suspend" };

	CommunityConnection::LevelUploadListener::Listener levelUploadListener;
	CommunityConnection::LevelDownloadListener::Listener levelDownloadListener;
	CommunityConnection::LevelsQueriedListener::Listener levelQueryListener;

	SapphireRandom randomer;
	AutoResource<audio::AudioManager> audioManager = userapp::audioManager;
	bool audioManagerLoaded = false;
	FixedString backgroundMusicName = nullptr;
	Resource<audio::SoundClip> backgroundMusic;
	audio::SoundPlayerToken backgroundMusicToken;
	ArrayList<FixedString> availableMusicNames;
	const char** availableMusicItems = nullptr;

	AutoResource<RandomContext> randomContext;
	Randomer* uuidRandomer = nullptr;

	SapphireUser currentUser;

	LinkedList<FixedString, false> backgroundLoadingTexts;

	bool threadCancel = false;
	ContainerLinkedNode<FixedString> levelLoaderLoadingText = "Loading levels...";
	AsynchronTask* levelLoaderTask = nullptr;

	CommunityConnection communityConnection;

	bool lastInteractionKeyboard = false;
	bool keyboardDetected = false;
	bool mouseDetected = false;

	SapphireUILayer* topSapphireLayer = nullptr;

	SapphireBackgroundLayer* backgroundLayer = nullptr;

	ProgressSynchId localProgressSynchId = 0;

	Vector2F lastTouchPosition { -1, -1 };

	LevelStatistics totalStatistics;

	ArrayList<FixedString> levelPacks;

	class GamePadReference {
	public:
		GamePad* gamepad = nullptr;
		GamePadState state;
		GamePadKeyRepeater repeater;

		GamePadReference() {
			state.reset();
		}

		GamePadReference& operator=(NULLPTR_TYPE) {
			gamepad = nullptr;
			repeater.reset();
			return *this;
		}
		GamePadReference& operator=(GamePad* gp);
		GamePadReference& operator=(GamePadReference&& o) {
			this->gamepad = util::move(o.gamepad);
			o.gamepad = nullptr;
			this->state = util::move(o.state);
			this->repeater = util::move(o.repeater);
			return *this;
		}

		bool operator==(NULLPTR_TYPE) const {
			return gamepad == nullptr;
		}
		bool operator!=(NULLPTR_TYPE) const {
			return gamepad != nullptr;
		}

		GamePad* operator->() {
			ASSERT(gamepad != nullptr);
			return gamepad;
		}

		operator GamePad*() {
			ASSERT(gamepad != nullptr);
			return gamepad;
		}

	};
	GamePadReference playerGamePads[SAPPHIRE_MAX_PLAYER_COUNT] {};

	int argc = 0;
	char** argv = nullptr;

	unsigned int getFinishedLevelCount() {
		unsigned int res = 0;
		for (unsigned int p = 0; p < 2; ++p) {
			for (unsigned int c : finishedStatistics[p]) {
				res += c;
			}
		}
		return res;
	}

	unsigned int getFinishedLevelCount(SapphireDifficulty difficulty) {
		unsigned int result = 0;
		for (unsigned int pc = 0; pc < 2; ++pc) {
			result += finishedStatistics[pc][(unsigned int) difficulty];
		}
		return result;
	}

	void sortLevels(ArrayList<SapphireLevelDescriptor>& levelarray);

	unsigned int loadSettings();

	virtual void onPlayerStopped() override;

	void updateCustomLevelUUIDs();
	void updateLevelDemosWithUUID();
	void updateLevelStatesWithUUID();
	void upgradeVersionCollectProgress();

	template<unsigned int Version>
	void upgradeVersion();

	void upgradeMoveDataFiles();

	void dispatchPostLoadVersionUpgrade(unsigned int version);
	template<unsigned int Version>
	void upgradeVersionPostLoad();

	virtual void onLoggedIn(CommunityConnection* connection) override;

	template<typename Task>
	void addLaterLoginTask(Task&& task) {
		if (communityConnection.isLoggedIn()) {
			task();
		} else {
			loginConnectionTasks += *LoginConnectionTask::new_listener(util::forward<Task&&>(task));
		}
	}

	template<typename Task>
	void addLoginTask(Task&& task) {
		if (communityConnection.isLoggedIn()) {
			task();
		} else {
			loginConnectionTasks += *LoginConnectionTask::new_listener(util::forward<Task&&>(task));
			communityConnection.connect(this);
		}
	}

	void appendLevelState(const SapphireUUID& level, LevelState state);

	void writeLevelProgress(const SapphireLevelDescriptor* desc, uint32 randomseed, const FixedString& steps,
			SapphireLevelCommProgress progress);
	void writeTimeLevelProgress(const SapphireUUID& level, uint32 timeplayed);

	ProgressSynchId countProgressSynchId();

	int getUnknownLevelIndex(const SapphireUUID& uuid) {
		for (unsigned int i = 0; i < unknownLevelStates.size(); ++i) {
			if (unknownLevelStates.get(i)->levelUUID == uuid) {
				return i;
			}
		}
		return -1;
	}

	virtual void onGamePadAttached(GamePad* gamepad) override;
	virtual void onGamePadDetached(GamePad* gamepad) override;

	void initGamePads();
	void destroyGamePads();

	template<KeyCode Key>
	void sendGamePadKeyMessage(GamePadReference& ref, GamePadButtons changedbuttons, GamePadButtons checkbutton,
			GamePadButtons currentbuttons, core::time_millis time);
	template<KeyCode Key>
	void sendGamePadTriggerKeyMessage(GamePadReference& ref, uint32 val, uint32 prevval, uint32 max, uint32 deadzone,
			core::time_millis time);

	virtual void onTimeChanged(const core::time_millis& time, const core::time_millis& previous) override;

#if defined(SAPPHIRE_STEAM_API_AVAILABLE)
	class SteamCallbackTimeListener: public TimeListener {
	public:
		SteamCallbackTimeListener() {
			core::GlobalMonotonicTimeListener::addListenerToEnd(*this);
		}
		virtual void onTimeChanged(const core::time_millis& time, const core::time_millis& previous) override {
			SteamAPI_RunCallbacks();
		}
	} steamCallbackListener;
	ArrayList<SapphireSteamAchievement> steamAchievements;
	bool appBorrowed = false;

	STEAM_CALLBACK(SapphireScene, onGameRichPresenceJoinRequested, GameRichPresenceJoinRequested_t);
	STEAM_CALLBACK(SapphireScene, onUserStatsReceived, UserStatsReceived_t);
	void prepareSteamAchievements();

	template<typename FunctionType, typename... Args>
	bool iterateAchievements(FunctionType func, ISteamUserStats* stats, Args&&... args);
	template<typename FunctionType, typename... Args>
	bool iterateAchievementsStore(FunctionType func, ISteamUserStats* stats, Args&&... args);
	void callUserProgressChangedAchievements() {
		if(auto* stats = SteamUserStats()) {
			iterateAchievementsStore(&SapphireSteamAchievement::onUserProgressScoreChanged, stats, this, getUserProgressScore());
		}
	}
	template<typename... Args>
	void callLevelStateChangedAchievements(Args&&... args) {
		if(auto* stats = SteamUserStats()) {
			iterateAchievementsStore(&SapphireSteamAchievement::onLevelStateChanged, stats, this, util::forward<Args>(args)...);
		}
	}
	void callTotalStatsChangedAchievements() {
		if(auto* stats = SteamUserStats()) {
			iterateAchievementsStore(&SapphireSteamAchievement::onTotalStatsChanged, stats, this, totalStatistics);
		}
	}
#else
	template<typename FunctionType, typename ... Args>
	bool iterateAchievements(FunctionType func, ISteamUserStats* stats, Args&&... args) {
		return false;
	}
	void callUserProgressChangedAchievements() {
	}
	template<typename ... Args>
	void callLevelStateChangedAchievements(Args&&... args) {
	}
	void callTotalStatsChangedAchievements() {
	}
#endif /* defined(SAPPHIRE_STEAM_API_AVAILABLE) */

	void performAsyncLoading(unsigned int version);
	void performLoadingFinish(unsigned int version);

	void readTotalStatistics();
	void writeTotalStatistics();

	FileDescriptor* createFileDescriptorForMusicName(const FixedString& name);
public:
	class SettinsChangedListener: public BasicListener<SettinsChangedListener> {
	public:
		virtual void onSettingsChanged(const SapphireSettings& settings) = 0;
	};
	class KeyMapChangedListener: public BasicListener<KeyMapChangedListener> {
	public:
		virtual void onKeyMapChanged(const SapphireKeyMap& keymap, const SapphireKeyMap& gamepadkeymap) = 0;
	};

	using LevelStateListener = SimpleListener<void(const SapphireLevelDescriptor*, LevelState)>;
	using UnknownLevelStateListener = SimpleListener<void(const SapphireUUID&, LevelState)>;
	using LevelProgressListener = SimpleListener<void(const SapphireLevelDescriptor* descriptor, LevelState state, const FixedString& steps, uint32 randomseed, const LevelStatistics* stats)>;
	using GamePadStateUpdatedListener = SimpleListener<void(GamePad* gamepad, unsigned int playerid, const GamePadState& prevstate, const GamePadState& currentstate, core::time_millis milliselapsed)>;

	LevelStateListener::Events levelStateListeners;
	UnknownLevelStateListener::Events unknownLevelStateListeners;
	SettinsChangedListener::Events settingsChangedListeners;
	KeyMapChangedListener::Events keyMapChangedListeners;
	LevelProgressListener::Events levelProgressEvents;
	GamePadStateUpdatedListener::Events gamepadStateUpdatedEvents;

	SapphireScene();
	~SapphireScene();

	SapphireCommError setRemoteLevelProgress(const SapphireUUID& level, LevelState state);

	LevelState getUnknownLevelState(const SapphireUUID& uuid);

	unsigned int getFinishedLevelCount(unsigned int plrcount, SapphireDifficulty difficulty) {
		return finishedStatistics[plrcount - 1][(unsigned int) difficulty];
	}
	unsigned int getSeenLevelCount(unsigned int plrcount, SapphireDifficulty difficulty) {
		return seenStatistics[plrcount - 1][(unsigned int) difficulty];
	}
	unsigned int getLevelCount(unsigned int plrcount, SapphireDifficulty difficulty) {
		return levels[plrcount - 1][(unsigned int) difficulty].size();
	}

	unsigned int getFinishedLevelCount(unsigned int plrcount) {
		unsigned int result = 0;
		for (unsigned int i = 0; i < (unsigned int) SapphireDifficulty::_count_of_entries; ++i) {
			result += finishedStatistics[plrcount - 1][i];
		}
		return result;
	}
	unsigned int getSeenLevelCount(unsigned int plrcount) {
		unsigned int result = 0;
		for (unsigned int i = 0; i < (unsigned int) SapphireDifficulty::_count_of_entries; ++i) {
			result += seenStatistics[plrcount - 1][i];
		}
		return result;
	}
	unsigned int getLevelCount(unsigned int plrcount) {
		unsigned int result = 0;
		for (unsigned int i = 0; i < (unsigned int) SapphireDifficulty::_count_of_entries; ++i) {
			result += levels[plrcount - 1][i].size();
		}
		return result;
	}

	Layer* touch() override;

	void setLastInteractionKeyboard(bool keyboard);
	void setLastInteractionTouch(TouchAction action, const Vector2F& cursorpos);
	bool isLastInteractionKeyboard() const {
		return lastInteractionKeyboard;
	}

	bool isKeyboardDetected() const {
#if defined(SAPPHIRE_DUAL_PLAYER_AVAILABLE)
		return true;
#endif /* defined(SAPPHIRE_DUAL_PLAYER_AVAILABLE) */
		return keyboardDetected;
	}
	bool isMouseDetected() const {
		return mouseDetected;
	}

	bool hasRecognizedGamePadAttached();

	SapphireLevelDescriptor* getLevelWithName(const FixedString& name);
	SapphireLevelDescriptor* getLevelWithUUID(const SapphireUUID& uuid);

	Randomer* getUUIDRandomer();

	const SapphireLevelDescriptor* updateLevel(Level& level, const SapphireLevelDescriptor* desc);
	void removeLevel(const SapphireLevelDescriptor* desc);

	void checkLevelDemos();
	void checkLevelDemo(const SapphireLevelDescriptor* desc);
	bool isNeedBackground() const {
		return needBackground;
	}

	ArrayList<FixedString>& getLevelPackNames() {
		return levelPacks;
	}

	void addBackgroundLoadingText(LinkedNode<FixedString>& text) {
		backgroundLoadingTexts.addToEnd(text);
	}
	LinkedList<FixedString, false>& getBackgroundLoadingTexts() {
		return backgroundLoadingTexts;
	}
	bool isShowBackgroundLoading() const {
		return backgroundLoadingTexts.hasElements();
	}

	void setNeedBackground(bool needBackground) {
		this->needBackground = needBackground;
	}

	bool isLevelsLoaded() const {
		return levelLoaderTask == nullptr;
	}

	StorageDirectoryDescriptor& getUserLevelsDirectory() {
		return userLevelsDirectory;
	}

	SapphireLevelDescriptor** getDifficultyBegin(unsigned int plrcount, SapphireDifficulty diff) {
		ASSERT(plrcount >= 1 && plrcount <= 2) << plrcount;
		return levels[plrcount - 1][(unsigned int) diff].begin();
	}
	SapphireLevelDescriptor** getDifficultyEnd(unsigned int plrcount, SapphireDifficulty diff) {
		ASSERT(plrcount >= 1 && plrcount <= 2) << plrcount;
		return levels[plrcount - 1][(unsigned int) diff].end();
	}
	unsigned int getLevelCountByDifficulty(unsigned int plrcount, SapphireDifficulty diff) {
		return getDifficultyEnd(plrcount, diff) - getDifficultyBegin(plrcount, diff);
	}

	const SapphireSettings& getSettings() const {
		return settings;
	}
	void setSettings(const SapphireSettings& settings);
	void setKeyMap(const SapphireKeyMap& keymap);
	void setGamePadKeyMap(const SapphireKeyMap& keymap);
	const SapphireKeyMap& getKeyMap() const {
		return keyMap;
	}
	const SapphireKeyMap& getGamePadKeyMap() const {
		return gamepadKeyMap;
	}
	/**
	 * Does not write the settings to the disk
	 */
	void updateSettings(const SapphireSettings& settings);
	void writeSettings();

	void updateLevelState(const SapphireLevelDescriptor* desc, LevelState state, const FixedString& steps = nullptr, uint32 randomseed = 0,
			const LevelStatistics* stats = nullptr);
	void notifyLevelPlayed(const SapphireLevelDescriptor* desc, const Level& level);
	void updateLevelState(const SapphireUUID& uuid, LevelState state);
	void commitTimePlayedOnLevel(const SapphireLevelDescriptor* desc, uint32 time);
	void commitStatisticsPlayedOnLevel(const SapphireLevelDescriptor* desc, const LevelStatistics& stats);

	void setBackgroundMusic(const FixedString& musicname);

	void saveDemo(const SapphireLevelDescriptor* desc, const FixedString& name, unsigned int randomseed, const char* demodata,
			unsigned int datalength);
	void removeDemo(const Level& level, unsigned int demoindex);

	void loadCustomDemos(Level& level, const SapphireLevelDescriptor* desc);
	void loadCustomDemos(const SapphireLevelDescriptor* desc, ArrayList<Demo>& target);
	template<typename Handler>
	int loadCustomDemosHandler(const SapphireLevelDescriptor* desc, Handler&& handler);

	const SapphireLevelDescriptor* getNextLevel(const SapphireLevelDescriptor* desc) {
		SapphireDifficulty diff = desc->difficulty;
		for (auto* it = getDifficultyBegin(desc->playerCount, diff), *end = getDifficultyEnd(desc->playerCount, diff); it + 1 < end; ++it) {
			if (*it == desc) {
				return *(it + 1);
			}
		}
		diff = (SapphireDifficulty) ((unsigned int) diff + 1);
		while ((unsigned int) diff < (unsigned int) SapphireDifficulty::_count_of_entries) {
			if (getDifficultyBegin(desc->playerCount, diff) == getDifficultyEnd(desc->playerCount, diff)) {
				diff = (SapphireDifficulty) ((unsigned int) diff + 1);
			} else {
				return *getDifficultyBegin(desc->playerCount, diff);
			}
		}
		return nullptr;
	}

	const SapphireLevelDescriptor* getFirstUnfinishedLevel(SapphireDifficulty diff, int plrcount) {
		for (auto* it = getDifficultyBegin(plrcount, diff), *end = getDifficultyEnd(plrcount, diff); it != end; ++it) {
			if ((*it)->state != LevelState::COMPLETED) {
				return *it;
			}
		}
		return nullptr;
	}

	SapphireLevelDescriptor** findIterator(const SapphireLevelDescriptor* desc) {
		for (auto* it = getDifficultyBegin(desc->playerCount, desc->difficulty), *end = getDifficultyEnd(desc->playerCount,
				desc->difficulty); it != end; ++it) {
			if (*it == desc) {
				return it;
			}
		}
		return nullptr;
	}

	template<typename Handler>
	void iterateEveryLevel(Handler&& handler) {
		for (unsigned int pc = 0; pc < SAPPHIRE_MAX_PLAYER_COUNT; ++pc) {
			for (unsigned int i = 0; i < (unsigned int) SapphireDifficulty::_count_of_entries; ++i) {
				for (auto&& d : levels[pc][i]) {
					handler(d);
				}
			}
		}
	}

	virtual void setSceneManager(SceneManager* manager) override;

	virtual void onVisibilityToUserChanged(core::Window& window, bool visible) override;
	virtual void onInputFocusChanged(core::Window& window, bool inputFocused) override;

	virtual void onSizeChanged(core::Window& window, const core::WindowSize& size) override;

	core::WindowSize getUiSize();
	core::WindowSize getGameSize();
	core::WindowSize getRealSize() {
		return getWindow()->getWindowSize();
	}

	float getGameUserScale() const;
	float getUiUserScale() const;

	class LevelPlayPermission {
	public:
		SapphireDifficulty required;
		unsigned int completeCount;
	};

	bool isAllowedToPlay(SapphireDifficulty diff, LevelPlayPermission* outperm);
	SapphireDifficulty getMaxAllowedDifficulty();
	SapphireDifficulty getUserDifficultyColor();
	unsigned int getUserProgressScore();

	const SapphireUser& getCurrentUser() const {
		return currentUser;
	}
	const FixedString& getCurrentUserName();

	void setCurrentUserName(FixedString name);

	bool isLevelUploadable(const SapphireLevelDescriptor* desc);

	const SapphireLevelDescriptor* getDescriptorForDownloadedLevel(const Level& level);

	bool suspendLevel(const SapphireLevelDescriptor* desc, const Level& level);
	void deleteSuspendedLevel(const SapphireLevelDescriptor* desc);
	bool loadSuspendedLevel(const SapphireLevelDescriptor* desc, Level& level);

	CommunityConnection& getConnection() {
		return communityConnection;
	}

	void levelQueriedOnServer(const SapphireLevelDescriptor* desc);

	void setLevelUserRating(const SapphireLevelDescriptor* desc, unsigned int rating);

	SapphireUILayer* getTopSapphireLayer() const {
		return topSapphireLayer;
	}
	void setTopSapphireLayer(SapphireUILayer* layer) {
		this->topSapphireLayer = layer;
	}

	audio::AudioDescriptor* randomNewMusicAudio();
	audio::AudioDescriptor* createAudioDescriptorOrRandom();
	audio::AudioDescriptor* createAudioDescriptor();

	void setBackgroundLayer(SapphireBackgroundLayer* bg) {
		this->backgroundLayer = bg;
	}
	SapphireBackgroundLayer* getBackgroundLayer() {
		return this->backgroundLayer;
	}

	ProgressSynchId getProgressSynchId() const {
		return localProgressSynchId;
	}
	bool getLevelProgress(ProgressSynchId id, SapphireUUID* outleveluuid, FixedString* outsteps, uint32* outrandomseed,
			SapphireLevelCommProgress* outprogress);

	void navigateToStorePageToPurchase();

	LevelStatisticsDialog* showStatisticsDialogForLevel(SapphireUILayer* parent, const SapphireLevelDescriptor* level,
			const LevelStatistics& stats, const FixedString& steps, uint32 randomseed);
	LevelStatisticsDialog* showStatisticsDialogForLevel(SapphireUILayer* parent, const SapphireLevelDescriptor* level);

	LeaderboardsDialog* showLeaderboardsDialogForLevel(SapphireUILayer* parent, const SapphireLevelDescriptor* level);

	bool getLatestLevelFinishSteps(const SapphireLevelDescriptor* level, FixedString* outsteps, uint32* outrandomseed);

	int getPlayerIdForGamePad(GamePad* gp);
	GamePad* getGamePadForPlayerId(unsigned int playerid) {
		ASSERT(playerid < SAPPHIRE_MAX_PLAYER_COUNT) << playerid;
		return playerGamePads[playerid];
	}
	GamePadState* getGamePadStateForPlayerId(unsigned int playerid) {
		ASSERT(playerid < SAPPHIRE_MAX_PLAYER_COUNT) << playerid;
		return playerGamePads[playerid] == nullptr ? nullptr : &playerGamePads[playerid].state;
	}

	const LevelStatistics& getTotalStatistics() const {
		return totalStatistics;
	}

	StorageFileDescriptor& getRegistrationTokenFile() {
		return registrationTokenFile;
	}
	virtual void onDraw() override;

	void setProgramArguments(int argc, char** argv) {
		this->argc = argc;
		this->argv = argv;
	}
	int getProgramArgumentCount() const {
		return argc;
	}
	char** getProgramArguments() const {
		return argv;
	}

	/**
	 * Size is getAvailableMusicCount() + 1 with "Random Song" at the end
	 */
	const char* const * getAvailableMusicItems() const {
		return availableMusicItems;
	}
	unsigned int getAvailableMusicCount() const{
		return availableMusicNames.size();
	}
	unsigned int getMusicIndexForName(const FixedString& name) const;

	void updateServerProgressSynchId(ProgressSynchId serverid);

	bool canDownloadLevels() {
		return true;
	}

#if defined(SAPPHIRE_STEAM_API_AVAILABLE)
	void startPlayingLevelRequested(const char* uuid);
#endif /* defined(SAPPHIRE_STEAM_API_AVAILABLE) */

#if defined(SAPPHIRE_STEAM_API_AVAILABLE)
	bool isAppBorrowed() const {
		return appBorrowed;
	}
#endif /* defined(SAPPHIRE_STEAM_API_AVAILABLE) */
};

} // namespace userapp

#endif /* TEST_SAPPHIRE_SAPPHIRESCENE_H_ */
