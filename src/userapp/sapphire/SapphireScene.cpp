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
 * SapphireScene.cpp
 *
 *  Created on: 2016. apr. 23.
 *      Author: sipka
 */

#include <framework/render/RenderingContext.h>
#include <framework/threading/Thread.h>
#include <framework/io/files/AssetFileDescriptor.h>
#include <framework/xml/XmlAttributes.h>
#include <framework/audio/descriptor/OggVorbisAudioDescriptor.h>
#include <framework/io/files/StorageFileDescriptor.h>
#include <framework/scene/SceneManager.h>
#include <framework/core/Window.h>
#include <framework/io/stream/OutputStream.h>
#include <framework/io/stream/BufferedInputStream.h>
#include <framework/io/byteorder.h>
#include <framework/utils/ContainerLinkedNode.h>
#include <framework/io/gamepad/GamePad.h>
#include <framework/io/key/KeyMessage.h>
#include <framework/io/touch/TouchEvent.h>
#include <framework/message/Message.h>

#include <sapphire/SapphireScene.h>
#include <sapphire/level/Level.h>
#include <sapphire/AsynchronTask.h>
#include <sapphire/server/SapphireLevelDetails.h>
#include <sapphire/sapphireconstants.h>
#include <sapphire/dialogs/LevelStatisticsDialog.h>
#include <sapphire/dialogs/LeaderboardsDialog.h>
#include <sapphire/steam_opt.h>
#include <sapphire/dialogs/LevelDetailsLayer.h>
#include <sapphire/SapphireSteamAchievement.h>

#include <gen/xmldecl.h>
#include <gen/log.h>
#include <gen/fwd/types.h>
#include <gen/platform.h>

#include <string.h>

using namespace userapp;
using namespace rhfw;

LINK_XML_SIMPLE(SapphireScene)

namespace userapp {

class MusicAssetMapping {
public:
	const char* musicName;
	RAssetFile assetId;
};
static const MusicAssetMapping MUSIC_MAPPING[] { //
{ "A view to a Kill", rhfw::RAssets::gameres::game_sapphire::music::_000_A_view_to_a_kill_ogg }, //
		{ "Action", rhfw::RAssets::gameres::game_sapphire::music::_001_Action_ogg }, //
		{ "Axel F", rhfw::RAssets::gameres::game_sapphire::music::_002_Axel_f_ogg }, //
		{ "Calm", rhfw::RAssets::gameres::game_sapphire::music::_003_Calm_ogg }, //
		{ "Crystal", rhfw::RAssets::gameres::game_sapphire::music::_004_Crystal_ogg }, //
		{ "Dancing on the Ceiling", rhfw::RAssets::gameres::game_sapphire::music::_005_Dancing_on_the_ceiling_ogg }, //
		{ "Don't you forget about me", rhfw::RAssets::gameres::game_sapphire::music::Don_t_you_forget_about_me_ogg }, //
		{ "Downunder", rhfw::RAssets::gameres::game_sapphire::music::_006_Downunder_ogg }, //
		{ "Drive", rhfw::RAssets::gameres::game_sapphire::music::_007_Drive_ogg }, //
		{ "FF7choct", rhfw::RAssets::gameres::game_sapphire::music::_008_FF7choct_ogg }, //
		{ "Granite", rhfw::RAssets::gameres::game_sapphire::music::_009_Granite_ogg }, //
		{ "Imperial March", rhfw::RAssets::gameres::game_sapphire::music::Imperial_March_ogg }, //
		{ "Jump", rhfw::RAssets::gameres::game_sapphire::music::_010_Jump_ogg }, //
		{ "Just do it", rhfw::RAssets::gameres::game_sapphire::music::Just_do_it_ogg }, //
		{ "Live and let Die", rhfw::RAssets::gameres::game_sapphire::music::_011_Live_and_let_die_ogg }, //
		{ "Mission Impossible", rhfw::RAssets::gameres::game_sapphire::music::_012_Mission_impossible_ogg }, //
		{ "Nightshift", rhfw::RAssets::gameres::game_sapphire::music::_013_Nightshift_ogg }, //
		{ "Nobody does it Better", rhfw::RAssets::gameres::game_sapphire::music::Nobody_does_it_Better_ogg }, //
		{ "Only Solutions", rhfw::RAssets::gameres::game_sapphire::music::_014_Only_Solutions_ogg }, //
		{ "Oxygene4", rhfw::RAssets::gameres::game_sapphire::music::_015_Oxygene4_ogg }, //
		{ "Penny Lane", rhfw::RAssets::gameres::game_sapphire::music::_016_Penny_lane_ogg }, //
		{ "Perplexities", rhfw::RAssets::gameres::game_sapphire::music::Perplexities_ogg }, //
		{ "Pink Panther", rhfw::RAssets::gameres::game_sapphire::music::_017_Pink_panther_ogg }, //
		{ "Race", rhfw::RAssets::gameres::game_sapphire::music::_018_Race_ogg }, //
		{ "Ringing", rhfw::RAssets::gameres::game_sapphire::music::_019_Ringing_ogg }, //
		{ "Rooms", rhfw::RAssets::gameres::game_sapphire::music::Rooms_ogg }, //
		{ "Stars", rhfw::RAssets::gameres::game_sapphire::music::Stars_ogg }, //
		{ "Staying Alive", rhfw::RAssets::gameres::game_sapphire::music::_020_Staying_alive_ogg }, //
		{ "Take Five", rhfw::RAssets::gameres::game_sapphire::music::Take_Five_ogg }, //
		{ "Tico Tico", rhfw::RAssets::gameres::game_sapphire::music::_021_Tico_tico_ogg }, //
		{ "Time", rhfw::RAssets::gameres::game_sapphire::music::_022_Time_ogg }, //
		{ "Waterloo", rhfw::RAssets::gameres::game_sapphire::music::Waterloo_ogg }, //
		{ "We built this City", rhfw::RAssets::gameres::game_sapphire::music::We_built_this_City_ogg }, //
		{ "Winner", rhfw::RAssets::gameres::game_sapphire::music::_023_Winner_ogg }, //
		{ "Your Wildest Dreams", rhfw::RAssets::gameres::game_sapphire::music::Your_Wildest_Dreams_ogg }, //
};

FileDescriptor* SapphireScene::createFileDescriptorForMusicName(const FixedString& name) {
	if (name.length() == 0) {
		return nullptr;
	}
	for (auto&& map : MUSIC_MAPPING) {
		if (strcmp(map.musicName, (const char*) name) == 0) {
			return new AssetFileDescriptor(map.assetId);
		}
	}
	{
		StorageFileDescriptor desc { musicDirectory.getPath() + (const char*) name };
		if (desc.exists()) {
			return new StorageFileDescriptor(util::move(desc));
		}
	}
	{
		StorageFileDescriptor desc { musicDirectory.getPath() + (const char*) (name + ".ogg") };
		if (desc.exists()) {
			return new StorageFileDescriptor(util::move(desc));
		}
	}
	return nullptr;
}

static LevelState PROGRESS_TO_STATE_MAP[(unsigned int) SapphireLevelCommProgress::COMM_COUNT] { //
LevelState::UNSEEN, // Unknown
		LevelState::UNFINISHED, // Seen
		LevelState::COMPLETED, // Finished
		LevelState::UNFINISHED, // Seen_NoSteps
		LevelState::COMPLETED, // Finished_NoSteps
		LevelState::UNFINISHED, // TimePlayed
};
static SapphireLevelCommProgress STATE_TO_PROGRESS_MAP[(unsigned int) LevelState::_count_of_entries] { //
SapphireLevelCommProgress::Unknown, // UNSEEN
		SapphireLevelCommProgress::Seen, // UNFINISHED
		SapphireLevelCommProgress::Finished, // COMPLETED
};

LevelState sapphireCommProgressToState(SapphireLevelCommProgress progress) {
	ASSERT(progress < SapphireLevelCommProgress::COMM_COUNT);
	return PROGRESS_TO_STATE_MAP[(unsigned int) progress];
}
SapphireLevelCommProgress levelStateToSapphireCommProgress(LevelState state) {
	return STATE_TO_PROGRESS_MAP[(unsigned int) state];
}

#if defined(RHFW_PLATFORM_MACOSX)
#define KEYCODE_CMD_CTRL_REAL KeyCode::KEY_COMMAND
#define KEYCODE_CMD_CTRL_OTHER KeyCode::KEY_CTRL
#else
#define KEYCODE_CMD_CTRL_REAL KeyCode::KEY_CTRL
#define KEYCODE_CMD_CTRL_OTHER KeyCode::KEY_COMMAND
#endif /* defined(RHFW_PLATFORM_MACOSX) */

SapphireKeyMap::SapphireKeyMap()
		: keyCodeMap { //
//player 1 stuff
		KeyCode::KEY_DIR_UP, KeyCode::KEY_DIR_RIGHT, KeyCode::KEY_DIR_DOWN, KeyCode::KEY_DIR_LEFT, KeyCode::KEY_SHIFT,
		KEYCODE_CMD_CTRL_REAL,
		//player 2 stuff
		KeyCode::KEY_R, KeyCode::KEY_G, KeyCode::KEY_F, KeyCode::KEY_D,//
		KeyCode::KEY_Q, KeyCode::KEY_A,

		KeyCode::KEY_F2,//reset level
		KeyCode::KEY_Q,//editor paint
		KeyCode::KEY_F5,//quick-suspend

		KeyCode::KEY_NUM_ADD,// increase speed
		KeyCode::KEY_NUM_SUBTRACT,// decrease speed
	} {
	}
SapphireKeyMap::SapphireKeyMap(const gamepad_init& marker)
		: keyCodeMap {
				//player 1 stuff
				KeyCode::KEY_GAMEPAD_DPAD_UP, KeyCode::KEY_GAMEPAD_DPAD_RIGHT, KeyCode::KEY_GAMEPAD_DPAD_DOWN,
				KeyCode::KEY_GAMEPAD_DPAD_LEFT, //
				KeyCode::KEY_GAMEPAD_B,
				KeyCode::KEY_GAMEPAD_A,	//
				//player 2 stuff
				KeyCode::KEY_GAMEPAD_DPAD_UP, KeyCode::KEY_GAMEPAD_DPAD_RIGHT, KeyCode::KEY_GAMEPAD_DPAD_DOWN,
				KeyCode::KEY_GAMEPAD_DPAD_LEFT,	//
				KeyCode::KEY_GAMEPAD_B, KeyCode::KEY_GAMEPAD_A, //

				KeyCode::KEY_UNKNOWN, KeyCode::KEY_UNKNOWN, KeyCode::KEY_UNKNOWN, //

				KeyCode::KEY_UNKNOWN, KeyCode::KEY_UNKNOWN, //
		} {
}
void SapphireKeyMap::fixControlCommandKeys() {
	for (auto&& kc : keyCodeMap) {
		if (kc == KEYCODE_CMD_CTRL_OTHER) {
			kc = KEYCODE_CMD_CTRL_REAL;
		}
	}
}
//TODO create window style change event handler

static void applyWindowDisplayMode(core::Window* window, SapphireFullScreenState& mode) {
	LOGTRACE() << "Set display mode to: " << (unsigned int) mode;
	switch (mode) {
		case SapphireFullScreenState::WINDOWED: {
			if (renderer->getRenderingContext()->supportsExclusiveFullScreen()) {
				renderer->getRenderingContext()->setExclusiveFullScreen(window, false);
			}
			if (window->supportsWindowStyle(WindowStyle::FULLSCREEN | WindowStyle::BORDERED)) {
				window->setWindowStyle(WindowStyle::FULLSCREEN | WindowStyle::BORDERED);
			} else if (window->supportsWindowStyle(WindowStyle::BORDERED)) {
				window->setWindowStyle(WindowStyle::BORDERED);
			}
			break;
		}
		case SapphireFullScreenState::EXCLUSIVE_FULLSCREEN: {
			if (renderer->getRenderingContext()->supportsExclusiveFullScreen()) {
				if (window->supportsWindowStyle(WindowStyle::FULLSCREEN)) {
					window->setWindowStyle(WindowStyle::FULLSCREEN);
				}
				if (!renderer->getRenderingContext()->setExclusiveFullScreen(window, true)) {
					LOGE() << "Failed to set fullscreen mode";
					mode = SapphireFullScreenState::BORDERLESS_FULLSCREEN;
					applyWindowDisplayMode(window, mode);
				}
			} else {
				mode = SapphireFullScreenState::BORDERLESS_FULLSCREEN;
				applyWindowDisplayMode(window, mode);
			}
			break;
		}
		case SapphireFullScreenState::BORDERLESS_FULLSCREEN: {
			if (renderer->getRenderingContext()->supportsExclusiveFullScreen()) {
				renderer->getRenderingContext()->setExclusiveFullScreen(window, false);
			}
			if (window->supportsWindowStyle(WindowStyle::FULLSCREEN)) {
				window->setWindowStyle(WindowStyle::FULLSCREEN);
			}
			break;
		}
		default: {
			break;
		}
	}
#if defined(RHFW_PLATFORM_WIN32)
	window->setClipCursor(mode != SapphireFullScreenState::WINDOWED);
#endif /* defined(RHFW_PLATFORM_WIN32) */
}

void SapphireScene::checkLevelDemo(const SapphireLevelDescriptor* item) {
	if (item->serverSideAvailable) {
		return;
	}
	DemoPlayer player;
	WARN(item->demoCount == 0) << "No demo found for: " << item->title << " " << item->difficulty;
	Level level;
	for (unsigned int i = 0; i < item->demoCount; ++i) {
		WARN(item->leaderboards == SapphireLeaderboards::NO_FLAG) << "No leaderboard found for " << item->title << "  "
				<< item->difficulty;
		bool res = level.loadLevel(item->getFileDescriptor());
		ASSERT(res);
		//player.playFully(level.getDemo(i), level);
		player.play(level.getDemo(i), level);
		while (!level.isOver() && player.isPlaying()) {
			player.next(level);
			auto sc = level.getSoundCount();
			for (int si = 0; si < sc; ++si) {
				if (level.getGameSound(si).getSound() == SapphireSound::GameLost) {
					LOGW() << "Playing LOST sound for level: " << item->title << " in diff: " << item->difficulty << " Demo: "
							<< level.getDemo(i)->info.title << " at step: " << level.getTurn();
				}
			}
		}

		WARN(!level.isSuccessfullyOver()) << "Failed to complete level: " << item->title << " " << item->difficulty << " Demo: "
				<< level.getDemo(i)->info.title;
	}
}
void SapphireScene::checkLevelDemos() {
	LOGTRACE() << "Start level checking";
	for (int pc = 1; pc <= 2; ++pc) {
		for (int i = 0; i < (unsigned int) SapphireDifficulty::_count_of_entries; ++i) {
			for (auto item : levels[pc - 1][i]) {
				checkLevelDemo(item);
			}
		}
	}
	LOGTRACE() << "Level checking done.";
}

void SapphireScene::performAsyncLoading(unsigned int version) {
	FixedString* elliotpack = new FixedString("Elliot's classics");
	levelPacks.add(elliotpack);
	for (auto&& asset : RAssets::gameres::game_sapphire::levels::enumerate()) {
		if (threadCancel) {
			return;
		}
		AssetFileDescriptor fd { asset };
		auto* desc = SapphireLevelDescriptor::make(fd);
		ASSERT(desc != nullptr) << "Failed to load level " << asset;
		if (desc != nullptr) {
			desc->setFileDescriptor(new AssetFileDescriptor(util::move(fd)));
			levels[desc->playerCount - 1][(unsigned int) desc->difficulty].add(desc);
		}
	}
	for (auto&& asset : RAssets::gameres::game_sapphire::levels::custom::enumerate()) {
		if (threadCancel) {
			return;
		}
		AssetFileDescriptor fd { asset };
		auto* desc = SapphireLevelDescriptor::make(fd);
		ASSERT(desc != nullptr) << "Failed to load level " << asset;
		if (desc != nullptr) {
			desc->setFileDescriptor(new AssetFileDescriptor(util::move(fd)));
			desc->communityLevel = true;
			levels[desc->playerCount - 1][(unsigned int) desc->difficulty].add(desc);
		}
	}
	for (auto&& asset : RAssets::gameres::game_sapphire::levels::elliotclassic::enumerate()) {
		if (threadCancel) {
			return;
		}
		AssetFileDescriptor fd { asset };
		auto* desc = SapphireLevelDescriptor::make(fd);
		ASSERT(desc != nullptr) << "Failed to load level " << asset;
		if (desc != nullptr) {
#if !defined(SAPPHIRE_DUAL_PLAYER_AVAILABLE)
			if(desc->playerCount != 1) {
				delete desc;
				continue;
			}
#endif /* !defined(SAPPHIRE_DUAL_PLAYER_AVAILABLE) */
			desc->levelPack = *elliotpack;
			desc->setFileDescriptor(new AssetFileDescriptor(util::move(fd)));
			desc->communityLevel = true;
			levels[desc->playerCount - 1][(unsigned int) desc->difficulty].add(desc);
		}
	}
	{
#ifdef SAPPHIRE_DUAL_PLAYER_AVAILABLE
		for (auto&& asset : RAssets::gameres::game_sapphire::levels::twoplayer::enumerate()) {
			if (threadCancel) {
				return;
			}
			AssetFileDescriptor fd { asset };
			auto* desc = SapphireLevelDescriptor::make(fd);
			ASSERT(desc != nullptr) << "Failed to load level " << asset;
			if (desc != nullptr) {
				desc->setFileDescriptor(new AssetFileDescriptor(util::move(fd)));
				levels[desc->playerCount - 1][(unsigned int) desc->difficulty].add(desc);
			}
		}
		for (auto&& asset : RAssets::gameres::game_sapphire::levels::twoplayer::custom::enumerate()) {
			if (threadCancel) {
				return;
			}
			AssetFileDescriptor fd { asset };
			auto* desc = SapphireLevelDescriptor::make(fd);
			ASSERT(desc != nullptr) << "Failed to load level " << asset;
			if (desc != nullptr) {
				desc->setFileDescriptor(new AssetFileDescriptor(util::move(fd)));
				desc->communityLevel = true;
				levels[desc->playerCount - 1][(unsigned int) desc->difficulty].add(desc);
			}
		}
#endif /* defined(SAPPHIRE_DUAL_PLAYER_AVAILABLE) */
	}

	auto&& dowloadspath = downloadsDirectory.getPath();
	for (auto&& file : downloadsDirectory.enumerate()) {
		if (threadCancel) {
			return;
		}
		StorageFileDescriptor fd { dowloadspath + file };
		auto* desc = SapphireLevelDescriptor::make(fd);
		if (desc != nullptr) {
			if (desc->uuid) {
				desc->setFileDescriptor(new StorageFileDescriptor(util::move(fd)));
				desc->communityLevel = true;
				desc->locallyStoredLevel = true;
				desc->serverSideAvailable = true;
				levels[desc->playerCount - 1][(unsigned int) desc->difficulty].add(desc);
			} else {
				delete desc;
			}
		}
	}

	auto&& levelspath = getUserLevelsDirectory().getPath();
	for (auto&& file : getUserLevelsDirectory().enumerate()) {
		if (threadCancel) {
			return;
		}
		StorageFileDescriptor fd { levelspath + file };
		auto* desc = SapphireLevelDescriptor::make(fd);
		if (desc != nullptr) {
			if (desc->uuid && getLevelWithUUID(desc->uuid) == nullptr) {
				desc->setFileDescriptor(new StorageFileDescriptor(util::move(fd)));
				desc->communityLevel = true;
				desc->locallyStoredLevel = true;
				levels[desc->playerCount - 1][(unsigned int) desc->difficulty].add(desc);
			} else {
				delete desc;
			}
		}
	}

	for (int pc = 0; pc < 2; ++pc) {
		for (int i = 0; i < (unsigned int) SapphireDifficulty::_count_of_entries; ++i) {
			sortLevels(levels[pc][i]);
		}
	}

	if (threadCancel) {
		return;
	}

	auto&& suspendedpath = suspendedDirectory.getPath();
	for (auto&& file : suspendedDirectory.enumerate()) {
		if (threadCancel) {
			return;
		}
		StorageFileDescriptor fd { suspendedpath + file };
		auto istream = EndianInputStream<Endianness::Big>::wrap(LoopedInputStream::wrap(fd.openInputStream()));
		SapphireUUID leveluuid;
		if (!istream.deserialize<SapphireUUID>(leveluuid)) {
			continue;
		}
		auto* desc = getLevelWithUUID(leveluuid);
		if (desc != nullptr) {
			desc->hasSuspendedGame = true;
		}
	}

	{
		switch (version) {
			case 0:
			case 1: {
				auto* in = levelStatesFile.createInput();
				if (in->open()) {

					auto instream = EndianInputStream<Endianness::Host>::wrap(BufferedInputStream::wrap(*in));
					while (!threadCancel) {

						FixedString title;
						SapphireDifficulty diff;
						LevelState state;

						if (!instream.deserialize<SafeFixedString<SAPPHIRE_LEVEL_TITLE_MAX_LEN>>(title)) {
							break;
						}
						if (!instream.deserialize<uint32>(reinterpret_cast<uint32&>(diff))) {
							break;
						}
						if (!instream.deserialize<uint32>(reinterpret_cast<uint32&>(state))) {
							break;
						}

						/*TODO do with binary search*/
						for (int pc = 1; pc <= 2; ++pc) {
							for (auto* it = getDifficultyBegin(pc, diff), *end = getDifficultyEnd(pc, diff); it != end; ++it) {
								if ((*it)->title == title) {
									if (state > (*it)->state) {
										switch (state) {
											case LevelState::COMPLETED: {
												++finishedStatistics[(*it)->playerCount - 1][(unsigned int) diff];
												if ((*it)->state < LevelState::UNFINISHED) {
													++seenStatistics[(*it)->playerCount - 1][(unsigned int) diff];
												}
												break;
											}
											case LevelState::UNFINISHED: {
												++seenStatistics[(*it)->playerCount - 1][(unsigned int) diff];
												break;
											}
											default: {
												break;
											}
										}
										(*it)->state = state;
									}
									break;
								}
							}
						}
					}

					in->close();
				}
				delete in;
				break;
			}
			default: {
				auto* in = levelStatesFile.createInput();
				if (in->open()) {
					auto instream = EndianInputStream<Endianness::Host>::wrap(BufferedInputStream::wrap(*in));
					while (!threadCancel) {
						SapphireUUID uuid;
						LevelState state;
						if (!instream.deserialize<SapphireUUID>(uuid)) {
							break;
						}
						if (!instream.deserialize<uint32>(reinterpret_cast<uint32&>(state))) {
							break;
						}
						auto* desc = getLevelWithUUID(uuid);
						if (desc != nullptr) {
							if (state > desc->state) {
								switch (state) {
									case LevelState::COMPLETED: {
										++finishedStatistics[desc->playerCount - 1][(unsigned int) desc->difficulty];
										if (desc->state < LevelState::UNFINISHED) {
											++seenStatistics[desc->playerCount - 1][(unsigned int) desc->difficulty];
										}
										break;
									}
									case LevelState::UNFINISHED: {
										++seenStatistics[desc->playerCount - 1][(unsigned int) desc->difficulty];
										break;
									}
									default: {
										break;
									}
								}
								desc->state = state;
							}
						} else {
							int index = getUnknownLevelIndex(uuid);
							UnknownLevelState* unknown;
							if (index < 0) {
								unknown = new UnknownLevelState();
								unknown->levelUUID = uuid;
								unknownLevelStates.add(unknown);
							} else {
								unknown = unknownLevelStates.get(index);
							}
							unknown->state = state;

						}
					}
				}
				delete in;
				break;
			}
		}
	}

	localProgressSynchId = countProgressSynchId();
	LOGI() << "Local progress synch id: " << localProgressSynchId;

	levelDemosDirectory.create();
	downloadsDirectory.create();
	suspendedDirectory.create();

	readTotalStatistics();
}

static bool isFullASCII(const char* string) {
	bool onlyspace = true;
	while (*string != 0) {
		if (*string > 126 || *string < 32) {
			return false;
		}
		onlyspace = onlyspace && (*string == ' ');
		++string;
	}
	return !onlyspace;
}

void SapphireScene::performLoadingFinish(unsigned int version) {
	if (version != SAPPHIRE_RELEASE_VERSION_NUMBER) {
		dispatchPostLoadVersionUpgrade(version);
	}

	delete levelLoaderTask;
	levelLoaderTask = nullptr;
	levelLoaderLoadingText.removeLinkFromList();

	levelUploadListener =
			CommunityConnection::LevelUploadListener::make_listener(
					[=](SapphireCommError error, const SapphireUUID& uuid) {
						SapphireLevelDescriptor* desc = getLevelWithUUID(uuid);
						if(desc == nullptr || desc->serverSideAvailable) {
							return;
						}

						switch (error) {
							case SapphireCommError::LevelAlreadyExists:
							case SapphireCommError::LevelDemoRemoved:
							case SapphireCommError::NoError: {
								Level level;
								if(level.loadLevel(desc->getFileDescriptor())) {
									level.getInfo().nonModifyAbleFlag = true;
									StorageFileDescriptor* nfd = new StorageFileDescriptor(downloadsDirectory.getPath() + (const char*)desc->uuid.asString());
									level.saveLevel(*nfd);
									desc->getFileDescriptor().remove();

									desc->setFileDescriptor(nfd);
									desc->nonModifyAbleFlag = true;
									desc->serverSideAvailable = true;
								}
								break;
							}
							default: {
								break;
							}
						}
					});
	levelQueryListener = CommunityConnection::LevelsQueriedListener::make_listener(
			[=](unsigned int index, const SapphireLevelDetails* details) {
				if(details == nullptr) {
					return;
				}
				auto* desc = getLevelWithUUID(details->uuid);
				if(desc != nullptr) {
					desc->userRating = details->userRating;
				}
			});
	levelDownloadListener = CommunityConnection::LevelDownloadListener::make_listener(
			[=](const SapphireUUID& leveluuid, SapphireCommError error, const Level* level) {
				/*make sure the downloaded level has a descriptor, and is saved to disk*/
				if(level != nullptr) {
					getDescriptorForDownloadedLevel(*level);
				}
			});
	communityConnection.levelUploadEvents += levelUploadListener;
	communityConnection.levelDownloadEvents += levelDownloadListener;
	communityConnection.levelsQueriedEvents += levelQueryListener;
	if (this->canDownloadLevels()) {
		for (auto&& u : unknownLevelStates) {
			SapphireUUID uuid = u->levelUUID;
			addLaterLoginTask([=] {
				this->communityConnection.downloadLevel(uuid);
			});
		}
	}
#if defined(SAPPHIRE_STEAM_API_AVAILABLE)
	{
		auto* userstats = SteamUserStats();
		if (userstats != nullptr) {
			userstats->RequestCurrentStats();
		}
		auto* sf = SteamFriends();
		if (sf != nullptr && currentUser.getUserName().length() == 0) {
			auto* steamname = sf->GetPersonaName();
			if (isFullASCII(steamname)) {
				FixedString str { steamname };
				if (str
						!= currentUser.getUserName() && str.length() >= SAPPHIRE_USERNAME_MIN_LEN && str.length() <= SAPPHIRE_USERNAME_MAX_LEN) {
					currentUser.getUserName() = util::move(str);
					writeSettings();
				}
			}
		}
	}
#endif /* defined(SAPPHIRE_STEAM_API_AVAILABLE) */

	getConnection().connect(this);
}

SapphireScene::SapphireScene()
		: communityConnection(this), gamepadKeyMap(SapphireKeyMap::gamepad_init { }) {
	dataDirectory.create();

	randomer.setSeed((unsigned int) core::MonotonicTime::getCurrent());

	if (!EndianInputStream<Endianness::Big>::wrap(userUUIDFile.openInputStream()).deserialize<SapphireUUID>(currentUser.getUUID())) {
		currentUser.getUUID() = nullptr;
	}
	if (!currentUser.getUUID()) {
		getUUIDRandomer()->read(currentUser.getUUID().getData(), SapphireUUID::UUID_LENGTH);
		EndianOutputStream<Endianness::Big>::wrap(userUUIDFile.openOutputStream()).serialize<SapphireUUID>(currentUser.getUUID());
	}
	unsigned int version = loadSettings();
	communityConnection.addStateListener(*this);

	levelLoaderTask = new AsynchronTask([this, version]() {
		this->performAsyncLoading(version);
	}, [=] {
		this->performLoadingFinish(version);
	}, [=] {
		threadCancel = true;
		levelLoaderLoadingText.removeLinkFromList();
	});
	addBackgroundLoadingText(levelLoaderLoadingText);
	levelLoaderTask->start();

	audioManagerLoaded = audioManager.isLoaded();

	for (auto&& m : MUSIC_MAPPING) {
		availableMusicNames.add(new FixedString(m.musicName));
	}
	for (auto&& p : musicDirectory.enumerate()) {
		if (p.isDirectory()) {
			continue;
		}
		auto* str = new FixedString((const FilePathChar *) p);
		availableMusicNames.add(str);
		LOGTRACE() << "Found music: " << *str;

		//TODO check and remove .ogg extension
	}

	availableMusicItems = new const char*[availableMusicNames.size() + 1];
	for (unsigned int i = 0; i < availableMusicNames.size(); ++i) {
		auto&& str = availableMusicNames[i];
		auto * array = new char[str.length() + 1];

		availableMusicItems[i] = array;
		memcpy(array, (const char*) str, str.length());
		array[str.length()] = 0;
	}
	availableMusicItems[availableMusicNames.size()] = "Random Song";

	backgroundMusicName = settings.openingMusicName;

	if (settings.music > 0 && audioManagerLoaded) {
		backgroundMusic = audioManager->createSoundClip();
		auto* audiodesc = createAudioDescriptorOrRandom();
		backgroundMusic->setDescriptor(audiodesc);
		backgroundMusic.load();

		//background music playing will start when window becomes visible
		//backgroundMusicToken = audioManager->playSingle(backgroundMusic);
		//backgroundMusicToken.stoppedListeners += *this;
	}

	initGamePads();
}
SapphireScene::~SapphireScene() {
	destroyGamePads();
	if (levelLoaderTask != nullptr) {
		levelLoaderTask->cancel();
	}
	//clear the children, as still having children when super destructor runs, can result in crashes
	clearChildren();

	if (settings.music > 0) {
		backgroundMusicToken.stop();
		backgroundMusic.free();
	}

	core::WindowAccesStateListener::unsubscribe();

	delete uuidRandomer;

	delete levelLoaderTask;
	for (unsigned int i = 0; i < availableMusicNames.size(); ++i) {
		delete[] availableMusicItems[i];
	}
	delete[] availableMusicItems;
}

void SapphireScene::sortLevels(ArrayList<SapphireLevelDescriptor>& levelarray) {
	levelarray.sort([](const SapphireLevelDescriptor& a, const SapphireLevelDescriptor& b) {
		if (a.difficulty == b.difficulty) {
			if(a.communityLevel != b.communityLevel) {
				return !a.communityLevel;
			}
			return a.title < b.title;
		}
		return a.difficulty < b.difficulty;
	});
}
SapphireCommError SapphireScene::setRemoteLevelProgress(const SapphireUUID& level, LevelState state) {
	auto* desc = getLevelWithUUID(level);
	if (desc == nullptr) {
		int index = getUnknownLevelIndex(level);
		UnknownLevelState* unknownstate;
		if (index < 0) {
//non existent
			unknownstate = new UnknownLevelState();
			unknownstate->levelUUID = level;
			unknownstate->state = state;
			unknownLevelStates.add(unknownstate);
			for (auto&& l : unknownLevelStateListeners.foreach()) {
				l(level, state);
			}
			appendLevelState(level, state);
			if (this->canDownloadLevels()) {
				addLaterLoginTask([=] {
					this->communityConnection.downloadLevel(level);
				});
			}
		} else {
			unknownstate = unknownLevelStates.get(index);
			if (unknownstate->state < state) {
				unknownstate->state = state;
				for (auto&& l : unknownLevelStateListeners.foreach()) {
					l(level, state);
				}
				appendLevelState(level, state);
			}
//update the unknown state
			return SapphireCommError::NoError;
		}
		return SapphireCommError::LevelReadFailed;
	}
	if (desc->state >= state) {
		return SapphireCommError::NoError;
	}

	switch (state) {
		case LevelState::COMPLETED: {
			finishedStatistics[desc->playerCount - 1][(unsigned int) desc->difficulty]++;
			callUserProgressChangedAchievements();
			if (desc->state < LevelState::UNFINISHED) {
				seenStatistics[desc->playerCount - 1][(unsigned int) desc->difficulty]++;
			}
			break;
		}
		case LevelState::UNFINISHED: {
			seenStatistics[desc->playerCount - 1][(unsigned int) desc->difficulty]++;
			break;
		}
		default: {
			break;
		}
	}
	desc->state = state;
	for (auto&& l : levelStateListeners.foreach()) {
		l(desc, state);
	}
	callLevelStateChangedAchievements(desc);
	appendLevelState(level, state);
	return SapphireCommError::NoError;
}
void SapphireScene::updateLevelState(const SapphireUUID& uuid, LevelState state) {
	auto* desc = getLevelWithUUID(uuid);
	if (desc != nullptr) {
		updateLevelState(desc, state);
	}
}

void SapphireScene::onGamePadAttached(GamePad* gamepad) {
	LOGTRACE();
	for (auto&& gp : playerGamePads) {
		if (gp == nullptr) {
			gp = gamepad;
			if (!TimeListener::isSubscribed()) {
				core::GlobalMonotonicTimeListener::addListenerToEnd(*this);
			}
			break;
		}
	}
}
void SapphireScene::onGamePadDetached(GamePad* gamepad) {
	LOGTRACE();
	for (auto&& gp : playerGamePads) {
		if (gp == gamepad) {
			gp = nullptr;
			GamePadReference* current = &gp;
//move others forward
			for (unsigned int i = current - playerGamePads + 1; i < SAPPHIRE_MAX_PLAYER_COUNT; ++i) {
				if (playerGamePads[i] != nullptr) {
					*current++ = util::move(playerGamePads[i]);
				} else {
					break;
				}
			}
			break;
		}
	}
	if (!hasRecognizedGamePadAttached()) {
		TimeListener::unsubscribe();
	}
}
bool SapphireScene::hasRecognizedGamePadAttached() {
	for (auto&& gp : playerGamePads) {
		if (gp != nullptr) {
			return true;
		}
	}
	return false;
}

#if defined(RHFW_PLATFORM_WIN32) || defined(RHFW_PLATFORM_MACOSX) || defined(RHFW_PLATFORM_LINUX)
#define SHOW_CURSOR_VISIBLE(visible) getWindow()->setCursorVisible(visible)
#else
#define SHOW_CURSOR_VISIBLE(visible)
#endif /* defined(RHFW_PLATFORM_WIN32) */

void SapphireScene::setLastInteractionKeyboard(bool keyboard) {
	this->lastInteractionKeyboard = keyboard;
	keyboardDetected |= keyboard;

	SHOW_CURSOR_VISIBLE(!keyboard);
}
void SapphireScene::setLastInteractionTouch(TouchAction action, const Vector2F& cursorpos) {
	if (lastTouchPosition != cursorpos) {
		//just moved
		lastTouchPosition = cursorpos;
		//only show cursor when the position of it changed
		SHOW_CURSOR_VISIBLE(true);
		if (action == TouchAction::UP || action == TouchAction::DOWN) {
			this->lastInteractionKeyboard = false;
		}
	} else if (action == TouchAction::UP || action == TouchAction::DOWN || action == TouchAction::WHEEL || action == TouchAction::SCROLL) {
		this->lastInteractionKeyboard = false;
		SHOW_CURSOR_VISIBLE(true);
	}
}

Layer* SapphireScene::touch() {
	return Scene::touch();
}

const FixedString& SapphireScene::getCurrentUserName() {
	return currentUser.getUserName();
}

int SapphireScene::getPlayerIdForGamePad(GamePad* gamepad) {
	for (auto&& gp : playerGamePads) {
		if (gp == gamepad) {
			return &gp - playerGamePads;
		}
	}
	return -1;
}

void SapphireScene::initGamePads() {
	if (gamepadContext != nullptr) {
		gamepadContext->addGamePadStateListener(this);

		auto count = gamepadContext->getGamePadCount();
		int gpc = 0;
		for (unsigned int i = 0; i < count; ++i) {
			auto* gp = gamepadContext->getGamePad(i);
			playerGamePads[gpc++] = gp;
			if (gpc >= SAPPHIRE_MAX_PLAYER_COUNT) {
				break;
			}
		}
		if (gpc > 0) {
			core::GlobalMonotonicTimeListener::addListenerToEnd(*this);
		}
	}
}
void SapphireScene::destroyGamePads() {
	if (gamepadContext != nullptr) {
		gamepadContext->removeGamePadStateListener(this);
		for (auto&& gp : playerGamePads) {
			gp = nullptr;
		}
		TimeListener::unsubscribe();
	}
}
template<KeyCode Key>
void SapphireScene::sendGamePadKeyMessage(GamePadReference& ref, GamePadButtons changedbuttons, GamePadButtons checkbutton,
		GamePadButtons currentbuttons, core::time_millis time) {
	bool down = HAS_FLAG(currentbuttons, checkbutton);
	if (HAS_FLAG(changedbuttons, checkbutton)) {
		KeyMessage::ExtraData extra;
		extra.gamePad = ref;
		extra.keycode = Key;
		LOGI() << extra.keycode;
		KeyMessage::postMessage(getWindow(), InputDevice::GAMEPAD, down ? KeyAction::DOWN : KeyAction::UP, extra);
	}
	ref.repeater.update<Key>(getWindow(), down, time);
}
template<KeyCode Key>
void SapphireScene::sendGamePadTriggerKeyMessage(GamePadReference& ref, uint32 val, uint32 prevval, uint32 max, uint32 deadzone,
		core::time_millis time) {
	bool prevon = prevval > deadzone;
	bool on = val > deadzone;
	if (prevon != on) {
		KeyMessage::ExtraData extra;
		extra.gamePad = ref;
		extra.keycode = Key;
		LOGI() << extra.keycode;
		KeyMessage::postMessage(getWindow(), InputDevice::GAMEPAD, on ? KeyAction::DOWN : KeyAction::UP, extra);
	}
	ref.repeater.update<Key>(getWindow(), on, time);
}
void SapphireScene::onTimeChanged(const core::time_millis& time, const core::time_millis& previous) {
	if (!getWindow()->isInputFocused()) {
		return;
	}

	for (auto&& gp : playerGamePads) {
		if (gp != nullptr) {
			GamePadState state;
			if (!gp->getState(&state)) {
				continue;
			}
			GamePadButtons changedbuttons = state.buttons ^ gp.state.buttons;

			sendGamePadKeyMessage<KeyCode::KEY_GAMEPAD_A>(gp, changedbuttons, GamePadButtons::A, state.buttons, time);
			sendGamePadKeyMessage<KeyCode::KEY_GAMEPAD_B>(gp, changedbuttons, GamePadButtons::B, state.buttons, time);
			sendGamePadKeyMessage<KeyCode::KEY_GAMEPAD_X>(gp, changedbuttons, GamePadButtons::X, state.buttons, time);
			sendGamePadKeyMessage<KeyCode::KEY_GAMEPAD_Y>(gp, changedbuttons, GamePadButtons::Y, state.buttons, time);
			sendGamePadKeyMessage<KeyCode::KEY_GAMEPAD_BACK>(gp, changedbuttons, GamePadButtons::BACK, state.buttons, time);
			sendGamePadKeyMessage<KeyCode::KEY_GAMEPAD_START>(gp, changedbuttons, GamePadButtons::START, state.buttons, time);
			sendGamePadKeyMessage<KeyCode::KEY_GAMEPAD_DPAD_DOWN>(gp, changedbuttons, GamePadButtons::DPAD_DOWN, state.buttons, time);
			sendGamePadKeyMessage<KeyCode::KEY_GAMEPAD_DPAD_UP>(gp, changedbuttons, GamePadButtons::DPAD_UP, state.buttons, time);
			sendGamePadKeyMessage<KeyCode::KEY_GAMEPAD_DPAD_LEFT>(gp, changedbuttons, GamePadButtons::DPAD_LEFT, state.buttons, time);
			sendGamePadKeyMessage<KeyCode::KEY_GAMEPAD_DPAD_RIGHT>(gp, changedbuttons, GamePadButtons::DPAD_RIGHT, state.buttons, time);
			sendGamePadKeyMessage<KeyCode::KEY_GAMEPAD_LEFT_SHOULDER>(gp, changedbuttons, GamePadButtons::LEFT_SHOULDER, state.buttons,
					time);
			sendGamePadKeyMessage<KeyCode::KEY_GAMEPAD_LEFT_THUMB>(gp, changedbuttons, GamePadButtons::LEFT_THUMB, state.buttons, time);
			sendGamePadKeyMessage<KeyCode::KEY_GAMEPAD_RIGHT_SHOULDER>(gp, changedbuttons, GamePadButtons::RIGHT_SHOULDER, state.buttons,
					time);
			sendGamePadKeyMessage<KeyCode::KEY_GAMEPAD_RIGHT_THUMB>(gp, changedbuttons, GamePadButtons::RIGHT_THUMB, state.buttons, time);

			sendGamePadTriggerKeyMessage<KeyCode::KEY_GAMEPAD_LEFT_TRIGGER>(gp, state.triggerLeft, gp.state.triggerLeft,
					gp->getTriggerMax(), gp->getTriggerDeadzone(), time);
			sendGamePadTriggerKeyMessage<KeyCode::KEY_GAMEPAD_RIGHT_TRIGGER>(gp, state.triggerRight, gp.state.triggerRight,
					gp->getTriggerMax(), gp->getTriggerDeadzone(), time);
			for (auto&& l : gamepadStateUpdatedEvents.foreach()) {
				l(gp, &gp - playerGamePads, gp.state, state, time - previous);
			}
			gp.state = state;
		}
	}
}

SapphireScene::GamePadReference& SapphireScene::GamePadReference::operator=(GamePad* gp) {
	state.reset();
	gamepad = gp;
	repeater.setGamePad(gp);
	return *this;
}

void SapphireScene::updateLevelState(const SapphireLevelDescriptor* desc, LevelState astate, const FixedString& steps, uint32 randomseed,
		const LevelStatistics* stats) {
	if (astate <= LevelState::UNFINISHED && astate <= desc->state && steps.length() == 0) {
		//nothing changed, do not even update the progress
		return;
	}
	auto progress = levelStateToSapphireCommProgress(astate);
	writeLevelProgress(desc, randomseed, steps, progress);
	const_cast<SapphireLevelDescriptor*>(desc)->timePlayed += steps.length();

	for (auto&& l : levelProgressEvents.foreach()) {
		l(desc, astate, steps, randomseed, stats);
	}

	getConnection().notifyLevelProgress(localProgressSynchId, desc->uuid, steps, randomseed, progress);
	++localProgressSynchId;
	if (astate <= desc->state) {
		return;
	}
	switch (astate) {
		case LevelState::COMPLETED: {
			finishedStatistics[desc->playerCount - 1][(unsigned int) desc->difficulty]++;
			callUserProgressChangedAchievements();
			if (desc->state < LevelState::UNFINISHED) {
				seenStatistics[desc->playerCount - 1][(unsigned int) desc->difficulty]++;
			}
			break;
		}
		case LevelState::UNFINISHED: {
			seenStatistics[desc->playerCount - 1][(unsigned int) desc->difficulty]++;
			break;
		}
		default: {
			break;
		}
	}
	const_cast<SapphireLevelDescriptor*>(desc)->state = astate;
	for (auto&& l : levelStateListeners.foreach()) {
		l(desc, astate);
	}
	callLevelStateChangedAchievements(desc);

	appendLevelState(desc->uuid, desc->state);
}
void SapphireScene::writeTotalStatistics() {
	auto&& out = EndianOutputStream<Endianness::Big>::wrap(statisticsFile.openOutputStream());
	//version
	out.serialize<uint32>(1);
	totalStatistics.serializeSimple(out);
}
void SapphireScene::readTotalStatistics() {
	bool bres = true;
	auto&& in = EndianInputStream<Endianness::Big>::wrap(statisticsFile.openInputStream());
	uint32 version;
	bres = bres && in.deserialize<uint32>(version);
	if (!bres) {
		return;
	}
	switch (version) {
		case 1: {
			unsigned int playcount;
			bres = bres && totalStatistics.deserialize(in, &totalStatistics, &playcount);
			if (!bres) {
				//failed to read, reset
				totalStatistics = LevelStatistics { };
			}
			break;
		}
		default: {
			THROW();
			break;
		}
	}

}
void SapphireScene::commitStatisticsPlayedOnLevel(const SapphireLevelDescriptor* desc, const LevelStatistics& stats) {
	if (stats.turns == 0) {
		return;
	}
	totalStatistics += stats;
	callTotalStatsChangedAchievements();

	writeTotalStatistics();
}
void SapphireScene::commitTimePlayedOnLevel(const SapphireLevelDescriptor* desc, uint32 time) {
	if (time == 0) {
		return;
	}
	if (desc->state < LevelState::UNFINISHED) {
		updateLevelState(desc, LevelState::UNFINISHED);
	}
	//no need to keep track of the time played
//	getConnection().notifyLevelProgress(localProgressSynchId, desc->uuid, nullptr, 0, SapphireLevelCommProgress::TimePlayed);
//	++localProgressSynchId;
//	writeTimeLevelProgress(desc->uuid, time);
	const_cast<SapphireLevelDescriptor*>(desc)->timePlayed += time;
}
void SapphireScene::appendLevelState(const SapphireUUID& level, LevelState state) {
	auto ostream = EndianOutputStream<Endianness::Host>::wrap(levelStatesFile.openAppendStream());
	ostream.serialize<SapphireUUID>(level);
	ostream.serialize<uint32>((uint32) state);
}

audio::AudioDescriptor* SapphireScene::createAudioDescriptor() {
	FileDescriptor* filedesc = createFileDescriptorForMusicName(backgroundMusicName);
	if (filedesc != nullptr) {
		auto* audiodesc = audio::OggVorbisAudioDescriptor::create(filedesc);
		if (audiodesc != nullptr) {
			return audiodesc;
		}
		delete filedesc;
	}
	return nullptr;
}
audio::AudioDescriptor* SapphireScene::createAudioDescriptorOrRandom() {
	auto* res = createAudioDescriptor();
	if (res != nullptr) {
		return res;
	}
	return randomNewMusicAudio();
}
audio::AudioDescriptor* SapphireScene::randomNewMusicAudio() {
	while (true) {
		auto&& genname = availableMusicNames[randomer.next(availableMusicNames.size())];
		if (genname == backgroundMusicName) {
			continue;
		}
		FileDescriptor* filedesc = createFileDescriptorForMusicName(genname);
		if (filedesc != nullptr) {
			auto* audiodesc = audio::OggVorbisAudioDescriptor::create(filedesc);
			if (audiodesc == nullptr) {
				delete filedesc;
				continue;
			}
			backgroundMusicName = util::move(genname);
			return audiodesc;
		}
	}
}
unsigned int SapphireScene::getMusicIndexForName(const FixedString& name) const {
	for (unsigned int i = 0; i < availableMusicNames.size(); ++i) {
		if (availableMusicNames[i] == name) {
			return i;
		}
	}
	return availableMusicNames.size();
}

void SapphireScene::onPlayerStopped() {
	//background music stopped, rerandom and play new
	if (audioManagerLoaded) {
		audio::SoundPlayerToken::StoppedListener::unsubscribe();

		backgroundMusic.free();
		auto* audiofd = randomNewMusicAudio();
		backgroundMusic->setDescriptor(audiofd);
		backgroundMusic.load();

		backgroundMusicToken = audioManager->playSingle(backgroundMusic, settings.music / 100.0f);
		backgroundMusicToken.stoppedListeners += *this;
	}
}

void SapphireScene::updateCustomLevelUUIDs() {
	if (!currentUser.getUUID()) {
		getUUIDRandomer()->read(currentUser.getUUID().getData(), SapphireUUID::UUID_LENGTH);
	}
	EndianOutputStream<Endianness::Big>::wrap(userUUIDFile.openOutputStream()).serialize<SapphireUUID>(currentUser.getUUID());

	auto&& levelspath = getUserLevelsDirectory().getPath();
	for (auto&& file : getUserLevelsDirectory().enumerate()) {
		StorageFileDescriptor fd { levelspath + file };
		Level level;
		if (level.loadLevel(fd)) {
			if (!level.getInfo().uuid) {
				getUUIDRandomer()->read(level.getInfo().uuid.getData(), SapphireUUID::UUID_LENGTH);

				StorageFileDescriptor nfd { levelspath + (const char*) (level.getInfo().uuid.asString()) };
				level.updateLevelVersion();
				level.saveLevel(nfd);
			}
		}
		fd.remove();
	}
}
void SapphireScene::updateLevelDemosWithUUID() {
	auto&& levelspath = levelDemosDirectory.getPath();
	for (auto&& file : levelDemosDirectory.enumerate()) {
		StorageFileDescriptor fd { levelspath + file };
		auto* desc = getLevelWithName(FixedString { (const FilePathChar*) file });
		if (desc != nullptr) {
			StorageFileDescriptor target { levelspath + (const char*) desc->uuid.asString() };
			auto ostream = EndianOutputStream<Endianness::Big>::wrap(target.openOutputStream());
			unsigned int got;
			char* data = fd.readFully(&got);
			if (got > 0) {
				for (auto* ptr = data; ptr - data < got;) {
					//copied from old load demo code

					unsigned int randomseed = *reinterpret_cast<const unsigned int*>(ptr);
					ptr += sizeof(unsigned int);
					unsigned int datalen = *reinterpret_cast<const unsigned int*>(ptr);
					if (datalen > SAPPHIRE_DEMO_MAX_LEN) {
						break;
					}
					ptr += sizeof(unsigned int);
					const char* movesptr = ptr;
					ptr += datalen;

					unsigned int namelen = *reinterpret_cast<const unsigned int*>(ptr);
					ptr += sizeof(unsigned int);
					const char* nameptr = ptr;
					ptr += namelen;

					ostream.serialize<uint32>(randomseed);
					ostream.serialize<uint32>(namelen);
					ostream.write(nameptr, namelen);
					ostream.serialize<uint32>(datalen);
					ostream.write(movesptr, datalen);
				}
			}
			fd.remove();
		}
	}
}
void SapphireScene::updateLevelStatesWithUUID() {
	StorageFileDescriptor newfile { StorageDirectoryDescriptor::Root() + "temporary" };
	{
		auto stream = EndianOutputStream<Endianness::Host>::wrap(newfile.openOutputStream());

		for (unsigned int pc = 0; pc < 2; ++pc) {
			for (unsigned int diff = 0; diff < (unsigned int) SapphireDifficulty::_count_of_entries; ++diff) {
				for (auto&& desc : levels[pc][diff]) {
					if (desc->state != LevelState::UNSEEN) {
						stream.serialize<SapphireUUID>(desc->uuid);
						stream.serialize<uint32>((uint32) desc->state);
					}
				}
			}
		}
	}
	levelStatesFile.remove();
	newfile.move(levelStatesFile);
}
static void MoveDirectoryContents(StorageDirectoryDescriptor& dir, StorageDirectoryDescriptor& target) {
	target.create();
	auto&& path = dir.getPath();
	auto&& targetpath = target.getPath();
	for (auto&& f : dir.enumerate()) {
		StorageFileDescriptor file { path + f };
		StorageFileDescriptor target { targetpath + f };
		bool res = file.move(target);
		ASSERT(res);
	}
	dir.remove();
}
void SapphireScene::upgradeMoveDataFiles() {
	dataDirectory.create();
	MoveDirectoryContents(levelDemosDirectory_old, levelDemosDirectory);
	MoveDirectoryContents(userLevelsDirectory_old, userLevelsDirectory);
	MoveDirectoryContents(downloadsDirectory_old, downloadsDirectory);
	MoveDirectoryContents(suspendedDirectory_old, suspendedDirectory);

	levelStatesFile_old.move(levelStatesFile);
	registrationTokenFile_old.move(registrationTokenFile);
}
template<>
void SapphireScene::upgradeVersion<0>() {
	LOGTRACE() << "Upgrade Sapphire from version: 0";
	updateCustomLevelUUIDs();
	if (!settings.hardwareUUID) {
		getUUIDRandomer()->read(settings.hardwareUUID.getData(), SapphireUUID::UUID_LENGTH);
	}
	writeSettings();
	upgradeMoveDataFiles();
}
template<>
void SapphireScene::upgradeVersion<1>() {
	LOGTRACE() << "Upgrade Sapphire from version: 1";
	updateCustomLevelUUIDs();
	if (!settings.hardwareUUID) {
		getUUIDRandomer()->read(settings.hardwareUUID.getData(), SapphireUUID::UUID_LENGTH);
	}
	writeSettings();
	upgradeMoveDataFiles();
}
template<>
void SapphireScene::upgradeVersion<2>() {
	LOGTRACE() << "Upgrade Sapphire from version: 2";
	EndianOutputStream<Endianness::Big>::wrap(userUUIDFile.openOutputStream()).serialize<SapphireUUID>(currentUser.getUUID());
	writeSettings();
	upgradeMoveDataFiles();
}
template<>
void SapphireScene::upgradeVersion<3>() {
	LOGTRACE() << "Upgrade Sapphire from version: 3";
	EndianOutputStream<Endianness::Big>::wrap(userUUIDFile.openOutputStream()).serialize<SapphireUUID>(currentUser.getUUID());
	writeSettings();
	upgradeMoveDataFiles();
}
template<>
void SapphireScene::upgradeVersion<4>() {
	LOGTRACE() << "Upgrade Sapphire from version: 4";
	EndianOutputStream<Endianness::Big>::wrap(userUUIDFile.openOutputStream()).serialize<SapphireUUID>(currentUser.getUUID());
	writeSettings();
	upgradeMoveDataFiles();
}
template<>
void SapphireScene::upgradeVersion<5>() {
	LOGTRACE() << "Upgrade Sapphire from version: 5";
	writeSettings();
}
template<>
void SapphireScene::upgradeVersion<SAPPHIRE_RELEASE_VERSION_NUMBER>() {
	THROW() << "invalid call";
}
template<>
void SapphireScene::upgradeVersionPostLoad<0>() {
	LOGTRACE() << "Upgrade Sapphire from version (post loading): 0";
	updateLevelDemosWithUUID();
	updateLevelStatesWithUUID();
	upgradeVersionCollectProgress();
}
template<>
void SapphireScene::upgradeVersionPostLoad<1>() {
	LOGTRACE() << "Upgrade Sapphire from version (post loading): 1";
	updateLevelDemosWithUUID();
	updateLevelStatesWithUUID();
	upgradeVersionCollectProgress();
}
template<>
void SapphireScene::upgradeVersionPostLoad<2>() {
	LOGTRACE() << "Upgrade Sapphire from version (post loading): 2";
	upgradeVersionCollectProgress();
}
template<>
void SapphireScene::upgradeVersionPostLoad<3>() {
	LOGTRACE() << "Upgrade Sapphire from version (post loading): 3";
	upgradeVersionCollectProgress();
}
template<>
void SapphireScene::upgradeVersionPostLoad<4>() {
	LOGTRACE() << "Upgrade Sapphire from version (post loading): 4";
	upgradeVersionCollectProgress();
}
template<>
void SapphireScene::upgradeVersionPostLoad<5>() {
	LOGTRACE() << "Upgrade Sapphire from version (post loading): 5";
}
template<>
void SapphireScene::upgradeVersionPostLoad<SAPPHIRE_RELEASE_VERSION_NUMBER>() {
	THROW() << "invalid call";
}

//version
//music
//sound
#define SETTINGS_LENGTH_V0 (1 + 2)
//bool use3d
//bool ignored
//gamescale
//uiscale
#define SETTINGS_LENGTH_V1 (SETTINGS_LENGTH_V0 + 4)

unsigned int SapphireScene::loadSettings() {
	auto* in = settingsFile.createInput();
	unsigned int version = 0;

	unsigned int got;
	auto* buffer = settingsFile.readFully(&got);
	delete in;
	if (got == 0) {
		goto no_previous_settings;
	}
	{
		if (got < 4) {
			goto no_previous_settings;
		}
		auto* bufptr = reinterpret_cast<const unsigned int*>(buffer);
		version = *bufptr++;
		if (version & 0xFF000000) {
//in version 0, 1 the settings file was in Host endian.
//to overcome this, change the byteorder if the versions first byte is in the msb
//on little endian systems this swaps the order
			version = byteorder::betoh(version);
		}
		SapphireSettings set;
		if (version < 2) {
			switch (version) {
				case 0: {
					if (got < sizeof(unsigned int) * SETTINGS_LENGTH_V0) {
						version = 0;
						goto no_previous_settings;
					}
					set.music = *bufptr++ != 0 ? 100 : 0;
					set.sound = *bufptr++ != 0 ? 100 : 0;
					this->settings = set;

					upgradeVersion<0>();
					break;
				}
				case 1: {
					if (got < sizeof(unsigned int) * SETTINGS_LENGTH_V1) {
						version = 0;
						goto no_previous_settings;
					}
					set.music = *bufptr++ != 0 ? 100 : 0;
					set.sound = *bufptr++ != 0 ? 100 : 0;
					bufptr++;
					set.artStyle = SapphireArtStyle::PERSPECTIVE_3D;
					set.artStyle = *bufptr++ != 0 ? SapphireArtStyle::PERSPECTIVE_3D : SapphireArtStyle::RETRO_2D;
					set.gameScale = (GameScale) *bufptr++;
					set.uiScale = (GameScale) *bufptr++;

					if ((unsigned int) set.gameScale < (unsigned int) GameScale::_count_of_entries
							&& (unsigned int) set.uiScale < (unsigned int) GameScale::_count_of_entries) {
						//input validation
						this->settings = set;
					}

					upgradeVersion<1>();
					break;
				}
				default: {
					THROW() << "Invalid version: " << version;
					version = 0;
					goto no_previous_settings;
				}
			}
		} else {
			switch (version) {
				case 2: {
					auto instream = EndianInputStream<Endianness::Big>::wrap(
							MemoryInput<const unsigned int> { bufptr, got - (unsigned int) sizeof(uint32) });
					bool music;
					bool sound;
					bool res;
					bool ignored;
					res = instream.deserialize<bool>(music);
					res = res && instream.deserialize<bool>(sound);
					res = res && instream.deserialize<uint32>(reinterpret_cast<uint32&>(set.artStyle));
					res = res && instream.deserialize<bool>(ignored);
					res = res && instream.deserialize<uint32>(reinterpret_cast<uint32&>(set.gameScale))
							&& (unsigned int) set.gameScale < (unsigned int) GameScale::_count_of_entries;
					res = res && instream.deserialize<uint32>(reinterpret_cast<uint32&>(set.uiScale))
							&& (unsigned int) set.uiScale < (unsigned int) GameScale::_count_of_entries;
					res = res && instream.deserialize<uint32>(reinterpret_cast<uint32&>(set.fullScreenState))
							&& (unsigned int) set.fullScreenState < (unsigned int) SapphireFullScreenState::_count_of_entries;
					res = res && instream.deserialize<SafeFixedString< SAPPHIRE_USERNAME_MAX_LEN>>(currentUser.getUserName());
					res = res && instream.deserialize<SapphireUUID>(currentUser.getUUID());
					res = res && instream.deserialize<SapphireUUID>(set.hardwareUUID);
					res = res && instream.deserialize<bool>(ignored);
					res = res && instream.deserialize<bool>(ignored);
					res = res && keyMap.deserialize(version, instream);
					if (res) {
						set.music = music ? 100 : 0;
						set.sound = sound ? 100 : 0;
						if (set.artStyle < SapphireArtStyle::MIN || set.artStyle > SapphireArtStyle::MAX) {
							set.artStyle = SapphireArtStyle::PERSPECTIVE_3D;
						}
						this->settings = set;
					} else {
						goto no_previous_settings;
					}
					upgradeVersion<2>();
					break;
				}
				case 3:
				case 4: {
					SapphireKeyMap keyboardkeys;
					auto instream = EndianInputStream<Endianness::Big>::wrap(
							MemoryInput<const unsigned int> { bufptr, got - (unsigned int) sizeof(uint32) });
					bool music;
					bool sound;
					bool res;
					bool ignored;
					res = instream.deserialize<bool>(music);
					res = res && instream.deserialize<bool>(sound);
					res = res && instream.deserialize<uint32>(reinterpret_cast<uint32&>(set.artStyle));
					res = res && instream.deserialize<bool>(ignored);
					res = res && instream.deserialize<uint32>(reinterpret_cast<uint32&>(set.gameScale))
							&& (unsigned int) set.gameScale < (unsigned int) GameScale::_count_of_entries;
					res = res && instream.deserialize<uint32>(reinterpret_cast<uint32&>(set.uiScale))
							&& (unsigned int) set.uiScale < (unsigned int) GameScale::_count_of_entries;
					res = res && instream.deserialize<uint32>(reinterpret_cast<uint32&>(set.fullScreenState))
							&& (unsigned int) set.fullScreenState < (unsigned int) SapphireFullScreenState::_count_of_entries;
					res = res && instream.deserialize<SafeFixedString< SAPPHIRE_USERNAME_MAX_LEN>>(currentUser.getUserName());
					res = res && instream.deserialize<SapphireUUID>(currentUser.getUUID());
					res = res && instream.deserialize<SapphireUUID>(set.hardwareUUID);
					res = res && instream.deserialize<bool>(ignored);
					res = res && instream.deserialize<bool>(ignored);
					res = res && keyboardkeys.deserialize(version, instream);
					uint32 oldmusicid;
					res = res && instream.deserialize<uint32>(oldmusicid);
					if (res) {
						set.music = music ? 100 : 0;
						set.sound = sound ? 100 : 0;
						if (oldmusicid >= (unsigned int) SapphireMusic::_count_of_entries) {
							set.openingMusicName = nullptr;
						}
						if (set.artStyle < SapphireArtStyle::MIN || set.artStyle > SapphireArtStyle::MAX) {
							set.artStyle = SapphireArtStyle::PERSPECTIVE_3D;
						}
						keyMap = keyboardkeys;
						this->settings = set;
					} else {
						goto no_previous_settings;
					}
					if (version == 3) {
						upgradeVersion<3>();
					} else {
						upgradeVersion<4>();
					}
					break;
				}
				case 5: {
					//steam release
					SapphireKeyMap keyboardkeys;
					SapphireKeyMap gamepadkeys;
					auto instream = EndianInputStream<Endianness::Big>::wrap(
							MemoryInput<const unsigned int> { bufptr, got - (unsigned int) sizeof(uint32) });
					bool res;
					bool ignored;
					res = instream.deserialize<uint32>(set.music);
					res = res && instream.deserialize<uint32>(set.sound);
					res = res && instream.deserialize<uint32>(reinterpret_cast<uint32&>(set.artStyle));
					res = res && instream.deserialize<bool>(ignored);
					res = res && instream.deserialize<uint32>(reinterpret_cast<uint32&>(set.gameScale))
							&& (unsigned int) set.gameScale < (unsigned int) GameScale::_count_of_entries;
					res = res && instream.deserialize<uint32>(reinterpret_cast<uint32&>(set.uiScale))
							&& (unsigned int) set.uiScale < (unsigned int) GameScale::_count_of_entries;
					res = res && instream.deserialize<uint32>(reinterpret_cast<uint32&>(set.fullScreenState))
							&& (unsigned int) set.fullScreenState < (unsigned int) SapphireFullScreenState::_count_of_entries;
					res = res && instream.deserialize<SafeFixedString< SAPPHIRE_USERNAME_MAX_LEN>>(currentUser.getUserName());
					res = res && instream.deserialize<SapphireUUID>(set.hardwareUUID);
					res = res && instream.deserialize<bool>(ignored);
					res = res && instream.deserialize<bool>(ignored);
					res = res && keyboardkeys.deserialize(version, instream);
					res = res && gamepadkeys.deserialize(version, instream);
					res = res && instream.deserialize<SafeFixedString< SAPPHIRE_MUSIC_NAME_MAX_LEN>>(set.openingMusicName);
					res = res && instream.deserialize<uint32>(set.soundYamYam);
					res = res && instream.deserialize<uint32>(set.soundLaser);
					res = res && instream.deserialize<uint32>(set.soundExplosion);
					res = res && instream.deserialize<uint32>(set.soundConvert);
					res = res && instream.deserialize<uint32>(set.soundPickUp);
					res = res && instream.deserialize<uint32>(set.soundDoorUsing);
					res = res && instream.deserialize<uint32>(set.soundFalling);
					res = res && instream.deserialize<uint32>(set.soundEnemies);
					res = res && instream.deserialize<uint32>(set.soundWheel);
					res = res && instream.deserialize<uint32>(set.soundGems);
					res = res && instream.deserialize<bool>(set.leftHandedOnScreenControls);
					res = res && instream.deserialize<bool>(set.steamAchievementProgressIndicatorEnabled);
					if (res) {
						if (set.artStyle < SapphireArtStyle::MIN || set.artStyle > SapphireArtStyle::MAX) {
							set.artStyle = SapphireArtStyle::PERSPECTIVE_3D;
						}
						keyMap = keyboardkeys;
						gamepadKeyMap = gamepadkeys;
						this->settings = set;
					} else {
						goto no_previous_settings;
					}
					upgradeVersion<5>();
					break;
				}
				case 6: {
					//increase decrease keys
					SapphireKeyMap keyboardkeys;
					SapphireKeyMap gamepadkeys;
					auto instream = EndianInputStream<Endianness::Big>::wrap(
							MemoryInput<const unsigned int> { bufptr, got - (unsigned int) sizeof(uint32) });
					bool res;
					bool ignored;
					res = instream.deserialize<uint32>(set.music);
					res = res && instream.deserialize<uint32>(set.sound);
					res = res && instream.deserialize<uint32>(reinterpret_cast<uint32&>(set.artStyle));
					res = res && instream.deserialize<bool>(ignored);
					res = res && instream.deserialize<uint32>(reinterpret_cast<uint32&>(set.gameScale))
							&& (unsigned int) set.gameScale < (unsigned int) GameScale::_count_of_entries;
					res = res && instream.deserialize<uint32>(reinterpret_cast<uint32&>(set.uiScale))
							&& (unsigned int) set.uiScale < (unsigned int) GameScale::_count_of_entries;
					res = res && instream.deserialize<uint32>(reinterpret_cast<uint32&>(set.fullScreenState))
							&& (unsigned int) set.fullScreenState < (unsigned int) SapphireFullScreenState::_count_of_entries;
					res = res && instream.deserialize<SafeFixedString< SAPPHIRE_USERNAME_MAX_LEN>>(currentUser.getUserName());
					res = res && instream.deserialize<SapphireUUID>(set.hardwareUUID);
					res = res && instream.deserialize<bool>(ignored);
					res = res && instream.deserialize<bool>(ignored);
					res = res && keyboardkeys.deserialize(version, instream);
					res = res && gamepadkeys.deserialize(version, instream);
					res = res && instream.deserialize<SafeFixedString< SAPPHIRE_MUSIC_NAME_MAX_LEN>>(set.openingMusicName);
					res = res && instream.deserialize<uint32>(set.soundYamYam);
					res = res && instream.deserialize<uint32>(set.soundLaser);
					res = res && instream.deserialize<uint32>(set.soundExplosion);
					res = res && instream.deserialize<uint32>(set.soundConvert);
					res = res && instream.deserialize<uint32>(set.soundPickUp);
					res = res && instream.deserialize<uint32>(set.soundDoorUsing);
					res = res && instream.deserialize<uint32>(set.soundFalling);
					res = res && instream.deserialize<uint32>(set.soundEnemies);
					res = res && instream.deserialize<uint32>(set.soundWheel);
					res = res && instream.deserialize<uint32>(set.soundGems);
					res = res && instream.deserialize<bool>(set.leftHandedOnScreenControls);
					res = res && instream.deserialize<bool>(set.steamAchievementProgressIndicatorEnabled);
					if (res) {
						if (set.artStyle < SapphireArtStyle::MIN || set.artStyle > SapphireArtStyle::MAX) {
							set.artStyle = SapphireArtStyle::PERSPECTIVE_3D;
						}
						keyMap = keyboardkeys;
						gamepadKeyMap = gamepadkeys;
						this->settings = set;
					} else {
						goto no_previous_settings;
					}
					break;
				}
				default: {
					THROW() << "Invalid version: " << version;
					version = 0;
					goto no_previous_settings;
				}
			}
			goto cleanup;
		}

	}
	no_previous_settings:

	{
		upgradeVersion<0>();
	}

	cleanup:

	settings.multiSampleFactor = renderer->getRenderingContextOptions().multiSamplingFactor;
	settings.vsync = renderer->getRenderingContextOptions().vsyncOptions != VSyncOptions::VSYNC_OFF;

	delete[] buffer;
	return version;
}
void SapphireScene::writeSettings() {
	auto outstream = EndianOutputStream<Endianness::Big>::wrap(settingsFile.openOutputStream());
	outstream.serialize<uint32>(SAPPHIRE_RELEASE_VERSION_NUMBER);

	outstream.serialize<uint32>(settings.music);
	outstream.serialize<uint32>(settings.sound);
	outstream.serialize<uint32>((uint32) settings.artStyle);
	outstream.serialize<bool>(true);
	outstream.serialize<uint32>((uint32) settings.gameScale);
	outstream.serialize<uint32>((uint32) settings.uiScale);
	outstream.serialize<uint32>((uint32) settings.fullScreenState);
	outstream.serialize<FixedString>(currentUser.getUserName());
	outstream.serialize<SapphireUUID>(settings.hardwareUUID);
	outstream.serialize<bool>(true);
	outstream.serialize<bool>(true);
	//TODO do with proper method
	keyMap.serialize(outstream);
	gamepadKeyMap.serialize(outstream);
	outstream.serialize<FixedString>(settings.openingMusicName);

	outstream.serialize<uint32>(settings.soundYamYam);
	outstream.serialize<uint32>(settings.soundLaser);
	outstream.serialize<uint32>(settings.soundExplosion);
	outstream.serialize<uint32>(settings.soundConvert);
	outstream.serialize<uint32>(settings.soundPickUp);
	outstream.serialize<uint32>(settings.soundDoorUsing);
	outstream.serialize<uint32>(settings.soundFalling);
	outstream.serialize<uint32>(settings.soundEnemies);
	outstream.serialize<uint32>(settings.soundWheel);
	outstream.serialize<uint32>(settings.soundGems);

	outstream.serialize<bool>(settings.leftHandedOnScreenControls);
	outstream.serialize<bool>(settings.steamAchievementProgressIndicatorEnabled);

	updateStartConfig(renderer->getRendererType(), renderer->getRenderingContextOptions().vsyncOptions,
			renderer->getRenderingContextOptions().multiSamplingFactor);

//	 auto* out = settingsFile.createOutput();
//	 out->open();
//
//	 unsigned int buffer[SETTINGS_LENGTH_V2];
//	 unsigned int* bufptr = buffer;
//	 *bufptr++ = SAPPHIRE_RELEASE_VERSION_NUMBER; //version
//	 *bufptr++ = settings.music ? 1 : 0;
//	 *bufptr++ = settings.sound ? 1 : 0;
//	 *bufptr++ = settings.use3d ? 1 : 0;
//	 bufptr++;
//	 *bufptr++ = (unsigned int) settings.gameScale;
//	 *bufptr++ = (unsigned int) settings.uiScale;
//	 *bufptr++ = (unsigned int) settings.fullScreenState;
//	 *bufptr++ = settings.vsync ? 1 : 0;
//	 *bufptr++ = settings.multiSampleFactor;
//
//	 out->write(buffer, sizeof(buffer));
//
//	 out->close();
//	 delete out;
}

void SapphireScene::updateSettings(const SapphireSettings& settings) {
	if (settings.music != this->settings.music) {
		if (audioManagerLoaded) {
			if (settings.music > 0) {
				if (this->settings.music == 0) {
					//turn on
					if (backgroundMusic == nullptr) {
						backgroundMusic = audioManager->createSoundClip();
					}
					auto* audiodesc = createAudioDescriptorOrRandom();
					backgroundMusic->setDescriptor(audiodesc);
					backgroundMusic.load();

					backgroundMusicToken = audioManager->playSingle(backgroundMusic, settings.music / 100.0f);
					backgroundMusicToken.stoppedListeners += *this;
				} else {
					if (backgroundMusicToken) {
						backgroundMusicToken.getPlayer()->setVolumeGain(settings.music / 100.0f);
					}
				}
			} else {
				//turn off
				backgroundMusicToken.stop();
				backgroundMusic.free();
			}
		}
	}
	bool globalScaleChanged = settings.uiScale != this->settings.uiScale;
	bool fschanged = settings.fullScreenState != this->settings.fullScreenState;

	auto renderopt = renderer->getRenderingContextOptions();

	if (this->settings.vsync != settings.vsync) {
		renderopt.vsyncOptions = settings.vsync ? VSyncOptions::VSYNC_ON : VSyncOptions::VSYNC_OFF;
	}

	if (this->settings.multiSampleFactor != settings.multiSampleFactor) {
		renderopt.multiSamplingFactor = settings.multiSampleFactor;
	}
	auto res = renderer->setRenderingContextOptions(renderopt);
	if (HAS_FLAG(res, RenderingContextOptionsResult::RELOAD_REQUIRED)) {
		renderer.reload();
	}

	this->settings = settings;
	if (fschanged) {
		applyWindowDisplayMode(getWindow(), this->settings.fullScreenState);
	}
	for (auto&& l : settingsChangedListeners.foreach()) {
		l.onSettingsChanged(settings);
	}
	if (globalScaleChanged) {
		Scene::sizeChanged(getUiSize());
	}
}
void SapphireScene::setSettings(const SapphireSettings& settings) {
	updateSettings(settings);
	writeSettings();
}
void SapphireScene::setKeyMap(const SapphireKeyMap& keymap) {
	this->keyMap = keymap;
	for (auto&& l : keyMapChangedListeners.foreach()) {
		l.onKeyMapChanged(keyMap, gamepadKeyMap);
	}
	writeSettings();
}
void SapphireScene::setGamePadKeyMap(const SapphireKeyMap& keymap) {
	this->gamepadKeyMap = keymap;
	for (auto&& l : keyMapChangedListeners.foreach()) {
		l.onKeyMapChanged(keyMap, gamepadKeyMap);
	}
	writeSettings();
}

void SapphireScene::setBackgroundMusic(const FixedString& musicname) {
	if (musicname == backgroundMusicName || musicname == nullptr) {
		return;
	}
	backgroundMusicName = musicname;

	if (settings.music > 0 && audioManagerLoaded) {
		auto* audiodesc = createAudioDescriptor();
		if (audiodesc == nullptr) {
			return;
		}
		backgroundMusicToken.stop();
		backgroundMusic.free();

		backgroundMusic->setDescriptor(audiodesc);
		backgroundMusic.load();

		backgroundMusicToken = audioManager->playSingle(backgroundMusic, settings.music / 100.0f);
		backgroundMusicToken.stoppedListeners += *this;
	}
}

void SapphireScene::saveDemo(const SapphireLevelDescriptor* desc, const FixedString& name, unsigned int randomseed, const char* demodata,
		unsigned int datalength) {
	LOGI() << "Saving demo for: " << desc->title << " with seed: " << randomseed << ": " << FixedString { demodata, datalength };
	StorageFileDescriptor fd { levelDemosDirectory.getPath() + (const char*) desc->uuid.asString() };

	auto ostream = EndianOutputStream<Endianness::Big>::wrap(fd.openAppendStream());
	ostream.serialize<uint32>(randomseed);
	ostream.serialize<uint32>(name.length());
	ostream.write((const char*) name, name.length());
	ostream.serialize<uint32>(datalength);
	ostream.write(demodata, datalength);
}
void SapphireScene::removeDemo(const Level& level, unsigned int demoindex) {
	ASSERT(level.getDemo(demoindex)->userDemo);
	Level nlevel { level };
	nlevel.removeDemo(demoindex);

	StorageFileDescriptor fd { levelDemosDirectory.getPath() + (const char*) nlevel.getInfo().uuid.asString() };
	auto ostream = EndianOutputStream<Endianness::Big>::wrap(fd.openOutputStream());

	for (int i = 0; i < nlevel.getDemoCount(); ++i) {
		const Demo* demo = nlevel.getDemo(i);
		if (demo->userDemo) {
			ostream.serialize<uint32>(demo->randomseed);
			ostream.serialize<uint32>(demo->info.title.length());
			ostream.write((const char*) demo->info.title, demo->info.title.length());
			ostream.serialize<uint32>(demo->moves.length());
			ostream.write((const char*) demo->moves, demo->moves.length());
		}
	}
}
template<typename Handler>
int SapphireScene::loadCustomDemosHandler(const SapphireLevelDescriptor* desc, Handler&& handler) {
	int count = 0;
	StorageFileDescriptor fd { levelDemosDirectory.getPath() + (const char*) desc->uuid.asString() };
	auto istream = EndianInputStream<Endianness::Big>::wrap(BufferedInputStream::wrap(fd.openInputStream()));
	while (true) {
		Demo* demo = new Demo();
		demo->userDemo = true;
		uint32 namelen;
		uint32 datalength;
		if (!istream.deserialize<uint32>(
				demo->randomseed)|| !istream.deserialize<uint32>(namelen) || namelen > SAPPHIRE_LEVEL_TITLE_MAX_LEN) {
			delete demo;
			break;
		}
		char* name = new char[namelen + 1];
		name[namelen] = 0;
		if (istream.read(name, namelen) < namelen) {
			delete demo;
			delete[] name;
			break;
		}
		demo->info.title = FixedString::make(name, namelen);
		if (!istream.deserialize<uint32>(datalength) || datalength > SAPPHIRE_DEMO_MAX_LEN) {
			delete demo;
			break;
		}
		char* demodata = new char[datalength + 1];
		demodata[datalength] = 0;
		if (istream.read(demodata, datalength) < datalength) {
			delete demo;
			delete[] demodata;
			break;
		}
		demo->moves = FixedString::make(demodata, datalength);
		handler(demo);
		++count;
	}
	return count;
}
void SapphireScene::loadCustomDemos(const SapphireLevelDescriptor* desc, ArrayList<Demo>& target) {
	loadCustomDemosHandler(desc, [&](Demo* demo) {
		target.add(demo);
	});

}
void SapphireScene::loadCustomDemos(Level& level, const SapphireLevelDescriptor* desc) {
	loadCustomDemosHandler(desc, [&](Demo* demo) {
		level.addDemo(demo);
	});
}

void SapphireScene::setSceneManager(SceneManager* manager) {
	Scene::setSceneManager(manager);
	manager->getWindow()->accesStateListeners += *this;
	applyWindowDisplayMode(manager->getWindow(), settings.fullScreenState);
}

void SapphireScene::onVisibilityToUserChanged(core::Window& window, bool visible) {
	if (settings.music > 0) {
		if (audioManagerLoaded) {
			if (!visible) {
				backgroundMusicToken.stop();
			} else {
				backgroundMusicToken = audioManager->playSingle(backgroundMusic, settings.music / 100.0f);
				backgroundMusicToken.stoppedListeners += *this;
			}
		}
	}
}

void SapphireScene::onInputFocusChanged(core::Window& window, bool inputFocused) {
	if (!inputFocused) {
		for (auto&& gp : playerGamePads) {
			if (gp != nullptr) {
				gp.state.reset();
				gp.repeater.clearInput();
			}
		}
	}
}

static float getDpiScale(GameScale scale) {
	switch (scale) {
		case GameScale::SMALLEST: {
			return 0.66667f;
		}
		case GameScale::SMALL: {
			return 0.8f;
		}
		case GameScale::BIG: {
			return 1.2f;
		}
		case GameScale::BIGGEST: {
			return 1.5f;
		}
		default: {
			return 1.0f;
		}
	}
}
static void applyWindowScale(GameScale scale, core::WindowSize& size) {
	size.dpi *= getDpiScale(scale);
}

void SapphireScene::onSizeChanged(core::Window& window, const core::WindowSize& size) {
	core::WindowSize localsize { size };
	applyWindowScale(settings.uiScale, localsize);
	Scene::onSizeChanged(window, localsize);
}

core::WindowSize SapphireScene::getUiSize() {
	core::WindowSize size = getWindow()->getWindowSize();
	applyWindowScale(settings.uiScale, size);
	return size;
}
core::WindowSize SapphireScene::getGameSize() {
	core::WindowSize size = getWindow()->getWindowSize();
	applyWindowScale(settings.gameScale, size);
	return size;
}

float SapphireScene::getGameUserScale() const {
	return getDpiScale(settings.gameScale);
}

const SapphireLevelDescriptor* SapphireScene::updateLevel(Level& level, const SapphireLevelDescriptor* desc) {
	ASSERT(desc == nullptr || desc->isEditable());

	SapphireLevelDescriptor* d;
	bool sort;
	unsigned int nplayercount = level.getMapPlayerCount();
	if (desc == nullptr) {
		//add new descriptor
		d = new SapphireLevelDescriptor();
		d->communityLevel = true;
		d->locallyStoredLevel = true;
		do {
			getUUIDRandomer()->read(d->uuid.getData(), SapphireUUID::UUID_LENGTH);
			//keep randoming until a non used uuid is found
		} while (getLevelWithUUID(d->uuid) != nullptr);
		d->setFileDescriptor(new StorageFileDescriptor { userLevelsDirectory.getPath() + (const char*) (d->uuid.asString()) });
		levels[nplayercount - 1][(unsigned int) level.getInfo().difficulty].add(d);
		sort = true;

		level.getInfo().uuid = d->uuid;
		level.getInfo().author = getCurrentUserName();
	} else {
		d = const_cast<SapphireLevelDescriptor*>(desc);
		sort = desc->title != level.getInfo().title;
		if (desc->difficulty != level.getInfo().difficulty || nplayercount != d->playerCount) {
			//change arraylist
			levels[d->playerCount - 1][(unsigned int) desc->difficulty].removeOne(d);
			levels[nplayercount - 1][(unsigned int) level.getInfo().difficulty].add(d);
			sort = true;
		}

	}

	d->playerCount = nplayercount;
	d->difficulty = level.getInfo().difficulty;
	d->category = level.getInfo().category;
	d->title = level.getInfo().title;
	d->demoCount = level.getDemoCount();

	if (sort) {
		sortLevels(levels[d->playerCount - 1][(unsigned int) d->difficulty]);
		updateLevelState(d, LevelState::UNSEEN);
	}
	return d;
}

void SapphireScene::removeLevel(const SapphireLevelDescriptor* desc) {
	ASSERT(desc->locallyStoredLevel);
	levels[desc->playerCount - 1][(unsigned int) desc->difficulty].removeOne(const_cast<SapphireLevelDescriptor*>(desc));
	delete desc;
}

float SapphireScene::getUiUserScale() const {
	return getDpiScale(settings.uiScale);
}

Randomer* SapphireScene::getUUIDRandomer() {
	if (randomContext == nullptr) {
		randomContext = Resource<RandomContext> { new ResourceBlock { new RandomContext { } } };
		uuidRandomer = randomContext->createRandomer();
	}
	return uuidRandomer;
}

void SapphireScene::dispatchPostLoadVersionUpgrade(unsigned int version) {
	switch (version) {
		case 0: {
			upgradeVersionPostLoad<0>();
			break;
		}
		case 1: {
			upgradeVersionPostLoad<1>();
			break;
		}
		case 2: {
			upgradeVersionPostLoad<2>();
			break;
		}
		case 3: {
			upgradeVersionPostLoad<3>();
			break;
		}
		case 4: {
			upgradeVersionPostLoad<4>();
			break;
		}
		case 5: {
			upgradeVersionPostLoad<5>();
			break;
		}
		default: {
			THROW() << "invalid version to upgrade from: " << version;
			break;
		}
	}
}

SapphireLevelDescriptor* SapphireScene::getLevelWithName(const FixedString& name) {
	//TODO should use binary search
	for (unsigned int pc = 0; pc < 2; ++pc) {
		for (unsigned int diff = 0; diff < (unsigned int) SapphireDifficulty::_count_of_entries; ++diff) {
			for (auto&& desc : levels[pc][diff]) {
				if (desc->title == name) {
					return desc;
				}
			}
		}
	}
	return nullptr;
}

SapphireLevelDescriptor* SapphireScene::getLevelWithUUID(const SapphireUUID& uuid) {
	//TODO should use binary search
	for (unsigned int pc = 0; pc < 2; ++pc) {
		for (unsigned int diff = 0; diff < (unsigned int) SapphireDifficulty::_count_of_entries; ++diff) {
			for (auto&& desc : levels[pc][diff]) {
				if (desc->uuid == uuid) {
					return desc;
				}
			}
		}
	}
	return nullptr;
}
unsigned int SapphireScene::getUserProgressScore() {
	unsigned int counter = 0;
	for (unsigned int p = 0; p < 2; ++p) {
		for (unsigned int i = 0; i < (unsigned int) SapphireDifficulty::_count_of_entries - 1; ++i) {
			counter += finishedStatistics[p][i] * (i + 1);
		}
		counter += finishedStatistics[p][(unsigned int) SapphireDifficulty::Unrated] * 2;
	}
	return counter;
}
const char* difficultyColorToSkillLevelName(SapphireDifficulty diff) {
	static const char* MAP[] {		//
	"Newbie",		//
			"Novice",		//
			"Beginner",		//
			"Advanced",		//
			"Apprentice",		//
			"Skilled",		//
			"Adept",		//
			"Expert",		//
			"Master",		//
			"Grandmaster", //
	};
	ASSERT(diff <= SapphireDifficulty::M_A_D_);
	return MAP[(unsigned int) diff];
}
SapphireDifficulty SapphireScene::getUserDifficultyColor() {
//	Novice
//	Beginner
//	Advanced
//	Apprentice
//	Skilled
//	Adept
//	Expert
//	Master
//	Grandmaster
	unsigned int counter = getUserProgressScore();
	if (counter <= SAPPHIRE_DIFFICULTY_SCORE_TUTORIAL) {
		return SapphireDifficulty::Tutorial;
	}
	if (counter <= SAPPHIRE_DIFFICULTY_SCORE_SIMPLE) {
		//Novice
		return SapphireDifficulty::Simple;
	}
	if (counter <= SAPPHIRE_DIFFICULTY_SCORE_EASY) {
		//Beginner
		return SapphireDifficulty::Easy;
	}
	if (counter <= SAPPHIRE_DIFFICULTY_SCORE_MODERATE) {
		//Advanced
		return SapphireDifficulty::Moderate;
	}
	if (counter <= SAPPHIRE_DIFFICULTY_SCORE_NORMAL) {
		//Apprentice
		return SapphireDifficulty::Normal;
	}
	if (counter <= SAPPHIRE_DIFFICULTY_SCORE_TRICKY) {
		//Skilled
		return SapphireDifficulty::Tricky;
	}
	if (counter <= SAPPHIRE_DIFFICULTY_SCORE_TOUGH) {
		//Adept
		return SapphireDifficulty::Tough;
	}
	if (counter <= SAPPHIRE_DIFFICULTY_SCORE_DIFFICULT) {
		//Expert
		return SapphireDifficulty::Difficult;
	}
	if (counter <= SAPPHIRE_DIFFICULTY_SCORE_HARD) {
		//Master
		return SapphireDifficulty::Hard;
	}
	//Grandmaster
	return SapphireDifficulty::M_A_D_;
}

SapphireDifficulty SapphireScene::getMaxAllowedDifficulty() {
	return SapphireDifficulty::M_A_D_;
}
bool SapphireScene::isAllowedToPlay(SapphireDifficulty diff, LevelPlayPermission* outperm) {
#if RHFW_DEBUG || SAPPHIRE_SCREENSHOT_MODE
	return true;
#else
	switch (diff) {
		case SapphireDifficulty::Tutorial: {
			return true;
		}
		case SapphireDifficulty::Unrated: {
			if (getFinishedLevelCount(SapphireDifficulty::Tutorial) >= 3) {
				return true;
			}
			outperm->required = SapphireDifficulty::Tutorial;
			outperm->completeCount = 3;
			return false;
		}
		default: {
			ASSERT((unsigned int ) diff > 0);
			if (!isAllowedToPlay((SapphireDifficulty) ((unsigned int) diff - 1), outperm)) {
				return false;
			}
			if (getFinishedLevelCount((SapphireDifficulty) (((unsigned int) diff) - 1)) >= 3) {
				return true;
			}
			outperm->required = (SapphireDifficulty) ((unsigned int) diff - 1);
			outperm->completeCount = 3;
			return false;
		}
	}
	THROW() << "Unreachable statement";
	return false;
#endif
}

void SapphireScene::setCurrentUserName(FixedString name) {
	this->currentUser.getUserName() = util::move(name);
	writeSettings();
	for (auto&& l : settingsChangedListeners.foreach()) {
		l.onSettingsChanged(settings);
	}
}

bool SapphireScene::isLevelUploadable(const SapphireLevelDescriptor* desc) {
	return desc->isEditable() && !desc->serverSideAvailable;
}

const SapphireLevelDescriptor* SapphireScene::getDescriptorForDownloadedLevel(const Level& level) {
	SapphireLevelDescriptor* desc = getLevelWithUUID(level.getInfo().uuid);
	if (desc != nullptr) {
		return desc;
	}
	desc = new SapphireLevelDescriptor(level);
	desc->serverSideAvailable = true;
	desc->setFileDescriptor(new StorageFileDescriptor { downloadsDirectory.getPath() + (const char*) (level.getInfo().uuid.asString()) });
	desc->userRating = 0;

	int index = getUnknownLevelIndex(desc->uuid);
	if (index >= 0) {
		auto* item = unknownLevelStates.remove(index);
		switch (item->state) {
			case LevelState::COMPLETED: {
				finishedStatistics[desc->playerCount - 1][(unsigned int) desc->difficulty]++;
				callUserProgressChangedAchievements();
				if (desc->state < LevelState::UNFINISHED) {
					seenStatistics[desc->playerCount - 1][(unsigned int) desc->difficulty]++;
				}
				break;
			}
			case LevelState::UNFINISHED: {
				seenStatistics[desc->playerCount - 1][(unsigned int) desc->difficulty]++;
				break;
			}
			default: {
				break;
			}
		}
		delete item;
		desc->state = item->state;
		for (auto&& l : levelStateListeners.foreach()) {
			l(desc, desc->state);
		}
		callLevelStateChangedAchievements(desc);
	}

	level.saveLevel(desc->getFileDescriptor(), true);

	levels[desc->playerCount - 1][(unsigned int) desc->difficulty].add(desc);
	sortLevels(levels[desc->playerCount - 1][(unsigned int) desc->difficulty]);
	return desc;
}

void SapphireScene::levelQueriedOnServer(const SapphireLevelDescriptor* descriptor) {
	SapphireLevelDescriptor* desc = const_cast<SapphireLevelDescriptor*>(descriptor);
	if (!desc->serverSideAvailable) {
		desc->serverSideAvailable = true;

		StorageFileDescriptor* nfd = new StorageFileDescriptor(downloadsDirectory.getPath() + (const char*) desc->uuid.asString());
		if (static_cast<StorageFileDescriptor&>(desc->getFileDescriptor()).move(*nfd)) {
			/*successfully moved*/
			desc->setFileDescriptor(nfd);

			desc->serverSideAvailable = true;
		}
	}
}
void SapphireScene::setLevelUserRating(const SapphireLevelDescriptor* desc, unsigned int rating) {
	ASSERT(rating >= 1 && rating <= 5) << rating;
	ASSERT(rating != desc->userRating);
	const_cast<SapphireLevelDescriptor*>(desc)->userRating = rating;
	addLoginTask([=] {
		communityConnection.rateLevel(desc->uuid, desc->userRating);
	});
}

bool SapphireScene::suspendLevel(const SapphireLevelDescriptor* desc, const Level& level) {
	if (level.getRecordedDemoLength() > SAPPHIRE_DEMO_MAX_LEN) {
		return false;
	}

	StorageFileDescriptor fd { suspendedDirectory.getPath() + (const char*) desc->uuid.asString() };
	auto ostream = EndianOutputStream<Endianness::Big>::wrap(fd.openOutputStream());
	bool res = ostream.serialize<SapphireUUID>(desc->uuid);
	res = res && ostream.serialize<uint32>(level.getOriginalRandomSeed());
	res = res && ostream.serialize<uint32>(level.getRecordedDemoLength());
	res = res && ostream.write(level.getRecordedDemo(), level.getRecordedDemoLength());

	const_cast<SapphireLevelDescriptor*>(desc)->hasSuspendedGame = res;

	return res;
}

void SapphireScene::deleteSuspendedLevel(const SapphireLevelDescriptor* desc) {
	StorageFileDescriptor fd { suspendedDirectory.getPath() + (const char*) desc->uuid.asString() };
	fd.remove();

	const_cast<SapphireLevelDescriptor*>(desc)->hasSuspendedGame = false;
}

bool SapphireScene::loadSuspendedLevel(const SapphireLevelDescriptor* desc, Level& level) {
	ASSERT(desc->hasSuspendedGame);
	StorageFileDescriptor fd { suspendedDirectory.getPath() + (const char*) desc->uuid.asString() };
	auto istream = EndianInputStream<Endianness::Big>::wrap(BufferedInputStream::wrap(LoopedInputStream::wrap(fd.openInputStream())));
	SapphireUUID leveluuid;
	uint32 randomseed;
	uint32 demolen;
	if (!istream.deserialize<SapphireUUID>(leveluuid) || !istream.deserialize<uint32>(randomseed) || !istream.deserialize<uint32>(demolen)
			|| leveluuid != desc->uuid || demolen > SAPPHIRE_DEMO_MAX_LEN || (demolen % level.getPlayerCount() != 0)) {
		return false;
	}
	char* data = new char[demolen];
	if (istream.read(data, demolen) != demolen) {
		delete[] data;
		return false;
	}

	level.setRandomSeed(randomseed);
	DemoPlayer::playMoves(data, demolen / level.getPlayerCount(), level);
	delete[] data;

	return true;
}

void SapphireScene::updateServerProgressSynchId(ProgressSynchId serverid) {
	//no longer used
//	ASSERT(localProgressSynchId < serverid);
//	auto&& ostream = EndianOutputStream<Endianness::Big>::wrap(progressFile.openAppendStream());
//
//	ostream.serialize<SapphireLevelCommProgress>(SapphireLevelCommProgress::IdOverride);
//	ostream.serialize<ProgressSynchId>(serverid);
//	localProgressSynchId = serverid;
}
ProgressSynchId SapphireScene::countProgressSynchId() {
	ProgressSynchId result = 0;

	auto&& istream = EndianInputStream<Endianness::Big>::wrap(progressFile.openInputStream());
	SapphireLevelCommProgress progress;
	SapphireUUID leveluuid;

	while (true) {
		long long position = istream.getPosition();

		if (!istream.deserialize<SapphireLevelCommProgress>(progress)) {
			break;
		}
		if (progress == SapphireLevelCommProgress::IdOverride) {
			//no longer used, but still deserialized to advance the stream
			ProgressSynchId id;
			if (!istream.deserialize<ProgressSynchId>(id)) {
				THROW();
				goto after_loop;
			}
//			result = id;
			continue;
		}
		if (!istream.deserialize<SapphireUUID>(leveluuid)) {
			break;
		}
		switch (progress) {
			case SapphireLevelCommProgress::Seen:
			case SapphireLevelCommProgress::Finished: {
				uint32 randomseed;
				uint32 stepslen;
				if (!istream.deserialize<uint32>(randomseed) || !istream.deserialize<LengthOnlyFixedString>(stepslen)) {
					THROW();
					goto after_loop;
				}
				auto* level = getLevelWithUUID(leveluuid);
				//level might be null, if we deleted a custom level
				if (level != nullptr) {
					LOGTRACE() << "Progress: " << result << ": " << level->getTitle() << " - " << progress;
					level->timePlayed += stepslen;
					if (progress == SapphireLevelCommProgress::Finished) {
						level->latestProgressFinishStepsOffset = position;
					}
				}
				break;
			}
			case SapphireLevelCommProgress::TimePlayed: {
				uint32 time;
				if (!istream.deserialize<uint32>(time)) {
					THROW();
					goto after_loop;
				}
				auto* level = getLevelWithUUID(leveluuid);
				//level might be null, if we deleted a custom level
				if (level != nullptr) {
					LOGTRACE() << "Progress: " << result << ": " << level->getTitle() << " - " << progress;
					level->timePlayed += time;
				}
				break;
			}
			case SapphireLevelCommProgress::Seen_NoSteps:
			case SapphireLevelCommProgress::Finished_NoSteps: {
				break;
			}
			default: {
				THROW() << progress;
				goto after_loop;
			}
		}
		++result;
	}
	after_loop:

	return result;
}
void SapphireScene::upgradeVersionCollectProgress() {
	auto&& ostream = EndianOutputStream<Endianness::Big>::wrap(progressFile.openOutputStream());

	ProgressSynchId localid = 0;
	LevelStatistics stats;

	for (unsigned int pc = 0; pc < 2; ++pc) {
		for (unsigned int diff = 0; diff < (unsigned int) SapphireDifficulty::_count_of_entries; ++diff) {
			for (auto&& desc : levels[pc][diff]) {
				if (desc->state != LevelState::UNSEEN) {
					++localid;
					if (desc->state == LevelState::COMPLETED) {
						int loadeddemos = loadCustomDemosHandler(desc, [&](Demo* d) {
							ostream.serialize<SapphireLevelCommProgress>(SapphireLevelCommProgress::Finished);
							ostream.serialize<SapphireUUID>(desc->uuid);
							ostream.serialize<uint32>(d->randomseed);
							ostream.serialize<FixedString>(d->moves);

							Level level;
							level.loadLevel(desc->getFileDescriptor());
							DemoPlayer::playMovesUntilSuccess(d->moves, d->moves.length() / level.getPlayerCount(), level);

							stats += level.getStatistics();
						});
						if (loadeddemos > 0) {
							continue;
						}
						ostream.serialize<SapphireLevelCommProgress>(SapphireLevelCommProgress::Finished_NoSteps);
						ostream.serialize<SapphireUUID>(desc->uuid);
					}
					//write simple progress as no demos were found
					ostream.serialize<SapphireLevelCommProgress>(SapphireLevelCommProgress::TimePlayed);
					ostream.serialize<SapphireUUID>(desc->uuid);
					ostream.serialize<uint32>(0);
				}
			}
		}
	}
	localProgressSynchId = localid;
	totalStatistics = stats;
	writeTotalStatistics();
}
void SapphireScene::writeLevelProgress(const SapphireLevelDescriptor* desc, uint32 randomseed, const FixedString& steps,
		SapphireLevelCommProgress progress) {
	ASSERT(progress == SapphireLevelCommProgress::Finished || progress == SapphireLevelCommProgress::Seen) << progress;

	if (progress == SapphireLevelCommProgress::Finished) {
		long long size = progressFile.size();
		if (size < 0) {
			size = 0;
		}
		const_cast<SapphireLevelDescriptor*>(desc)->latestProgressFinishStepsOffset = size;
		LOGI() << "Set latest finish steps offset " << desc->title << " - " << size;
	}

	auto&& ostream = EndianOutputStream<Endianness::Big>::wrap(progressFile.openAppendStream());
	ostream.serialize<SapphireLevelCommProgress>(progress);
	ostream.serialize<SapphireUUID>(desc->uuid);
	ostream.serialize<uint32>(randomseed);
	ostream.serialize<FixedString>(steps);
}
void SapphireScene::writeTimeLevelProgress(const SapphireUUID& level, uint32 timeplayed) {
	auto&& ostream = EndianOutputStream<Endianness::Big>::wrap(progressFile.openAppendStream());
	ostream.serialize<SapphireLevelCommProgress>(SapphireLevelCommProgress::TimePlayed);
	ostream.serialize<SapphireUUID>(level);
	ostream.serialize<uint32>(timeplayed);
}

LevelState SapphireScene::getUnknownLevelState(const SapphireUUID& uuid) {
	int index = getUnknownLevelIndex(uuid);
	if (index < 0) {
		return LevelState::UNSEEN;
	}
	return unknownLevelStates.get(index)->state;
}

bool SapphireScene::getLevelProgress(ProgressSynchId id, SapphireUUID* outleveluuid, FixedString* outsteps, uint32* outrandomseed,
		SapphireLevelCommProgress* outprogress) {
	auto&& istream = EndianInputStream<Endianness::Big>::wrap(progressFile.openInputStream());
	SapphireUUID leveluuid;
	uint32 randomseed;

	//TODO don't always read the whole file

	for (ProgressSynchId i = 0;; ++i) {
		if (!istream.deserialize<SapphireLevelCommProgress>(*outprogress)) {
			return false;
		}
		if (*outprogress == SapphireLevelCommProgress::IdOverride) {
			//no longer used but still deserialized to advance the stream
			ProgressSynchId nid;
			if (!istream.deserialize<ProgressSynchId>(nid)) {
				THROW();
				return false;
			}
//			i = nid - 1;

			//decrease i so the next one has the correct sequence
			--i;
			continue;
		}
		if (!istream.deserialize<SapphireUUID>(*outleveluuid)) {
			return false;
		}
		switch (*outprogress) {
			case SapphireLevelCommProgress::Seen:
			case SapphireLevelCommProgress::Finished: {
				if (i == id) {
					return istream.deserialize<uint32>(*outrandomseed)
							&& istream.deserialize<SafeFixedString<SAPPHIRE_DEMO_MAX_LEN>>(*outsteps);
				} else {
					if (!istream.deserialize<uint32>(randomseed) || !istream.deserialize<IgnoreFixedString>(nullptr)) {
						return false;
					}
				}
				break;
			}
			case SapphireLevelCommProgress::Seen_NoSteps:
			case SapphireLevelCommProgress::Finished_NoSteps: {
				if (i == id) {
					*outrandomseed = 0;
					*outsteps = nullptr;
					return true;
				}
				break;
			}
			case SapphireLevelCommProgress::TimePlayed: {
				uint32 time;
				if (!istream.deserialize<uint32>(time)) {
					return false;
				}
				if (i == id) {
					*outprogress = SapphireLevelCommProgress::Seen_NoSteps;
					*outrandomseed = 0;
					*outsteps = nullptr;
					return true;
				}
				break;
			}
			default: {
				THROW() << *outprogress;
				break;
			}
		}
	}
	return false;
}

void SapphireScene::onLoggedIn(CommunityConnection* connection) {
	for (auto&& p : loginConnectionTasks.pointers()) {
		(*p->get())();
		delete p;
	}
}

bool SapphireScene::getLatestLevelFinishSteps(const SapphireLevelDescriptor* descriptor, FixedString* outsteps, uint32* outrandomseed) {
	if (!descriptor->hasLatestFinishSteps()) {
		return false;
	}
	auto&& istream = EndianInputStream<Endianness::Big>::wrap(progressFile.openInputStream());
	LOGI() << "Skip progress to " << descriptor->latestProgressFinishStepsOffset;
	istream.skip(descriptor->latestProgressFinishStepsOffset);
	SapphireUUID leveluuid;
	uint32 randomseed;

	SapphireLevelCommProgress outprogress;
	SapphireUUID outleveluuid;
	if (!istream.deserialize<SapphireLevelCommProgress>(outprogress) || !istream.deserialize<SapphireUUID>(outleveluuid)) {
		THROW();
		return false;
	}
	if (outprogress != SapphireLevelCommProgress::Finished || outleveluuid != descriptor->uuid) {
		THROW() << outprogress << " - " << descriptor->title << " - " << outleveluuid.asString() << " != " << descriptor->uuid.asString();
		return false;
	}
	if (!istream.deserialize<uint32>(*outrandomseed) || !istream.deserialize<SafeFixedString<SAPPHIRE_DEMO_MAX_LEN>>(*outsteps)) {
		THROW();
		return false;
	}
	return true;
}

void SapphireScene::navigateToStorePageToPurchase() {
#if defined(SAPPHIRE_STEAM_API_AVAILABLE)
	auto* sf = SteamFriends();
	if (sf != nullptr) {
		sf->ActivateGameOverlayToStore(SAPPHIRE_STEAM_MAIN_APP_ID, k_EOverlayToStoreFlag_AddToCartAndShow);
	}
	return;
#endif /* defined(SAPPHIRE_STEAM_API_AVAILABLE) */
}

#if defined(SAPPHIRE_STEAM_API_AVAILABLE)
void SapphireScene::startPlayingLevelRequested(const char* uuid) {
	SapphireUUID leveluuid;
	const SapphireLevelDescriptor* descriptor;
	DialogLayer* dialog;
	if (SapphireUUID::fromString(&leveluuid, uuid)) {
		descriptor = getLevelWithUUID(leveluuid);
		if (descriptor == nullptr) {
			goto load_failed_label;
		}
		Level level;
		if (!level.loadLevel(descriptor->getFileDescriptor())) {
			goto load_failed_label;
		}
		dialog = new LevelDetailsLayer(getTopSapphireLayer(), descriptor, util::move(level));
	} else {
		load_failed_label:

		dialog = new DialogLayer(getTopSapphireLayer());
		dialog->setTitle("Loading failed");
		dialog->addDialogItem(new TextDialogItem("Failed to load requested level."));
		dialog->addDialogItem(new EmptyDialogItem(0.5f));
		dialog->addDialogItem(new CommandDialogItem("Back", [=] {
			dialog->dismiss();
		}));
	}
	dialog->showDialog(this);
}
void SapphireScene::onGameRichPresenceJoinRequested(GameRichPresenceJoinRequested_t* param) {
	startPlayingLevelRequested(param->m_rgchConnect);
}

void SapphireScene::onUserStatsReceived(UserStatsReceived_t* param) {
	if (param->m_nGameID != SAPPHIRE_STEAM_APP_ID) {
		LOGTRACE() << "Not sapphire app id for stats received " << (unsigned long long) param->m_nGameID;
		return;
	}
	if (param->m_eResult != k_EResultOK) {
		LOGTRACE() << "Error retrieving stats " << (unsigned long long) param->m_eResult;
		return;
	}
	auto* user = SteamUser();
	if (user == nullptr) {
		THROW() << "SteamUser() nullptr";
		return;
	}
	auto userid = user->GetSteamID();
	if (userid != param->m_steamIDUser) {
		LOGTRACE() << "Non-matching user id ";
		return;
	}
	auto* apps = SteamApps();
	if (apps == nullptr) {
		THROW() << "SteamApps() is nullptr";
		return;
	}
	appBorrowed = apps->GetAppOwner() != userid;
	class PrepareAchievementsMessage: public Message {
	protected:
		SapphireScene* scene;
		virtual bool dispatchMessageImpl() override {
			scene->prepareSteamAchievements();
			return true;
		}
	public:
		PrepareAchievementsMessage(SapphireScene* scene)
				: Message(), scene(scene) {
		}
	};
	(new PrepareAchievementsMessage(this))->post();
}
static const unsigned int SKILL_ACHIEVEMENTS_COUNT = 9;
static const char* ACHIEVEMENTS_SKILL[SKILL_ACHIEVEMENTS_COUNT] { //
"SKILL_NOVICE", //
		"SKILL_BEGINNER", //
		"SKILL_ADVANCED", //
		"SKILL_APPRENTICE", //
		"SKILL_SKILLED", //
		"SKILL_ADEPT", //
		"SKILL_EXPERT", //
		"SKILL_MASTER", //
		"SKILL_GRANDMASTER", //
};
static const unsigned int ACHIEVEMENTS_SKILL_THRESHOLDS[SKILL_ACHIEVEMENTS_COUNT] { //
SAPPHIRE_DIFFICULTY_SCORE_TUTORIAL, //
		SAPPHIRE_DIFFICULTY_SCORE_SIMPLE, //
		SAPPHIRE_DIFFICULTY_SCORE_EASY, //
		SAPPHIRE_DIFFICULTY_SCORE_MODERATE, //
		SAPPHIRE_DIFFICULTY_SCORE_NORMAL, //
		SAPPHIRE_DIFFICULTY_SCORE_TRICKY, //
		SAPPHIRE_DIFFICULTY_SCORE_TOUGH, //
		SAPPHIRE_DIFFICULTY_SCORE_DIFFICULT, //
		SAPPHIRE_DIFFICULTY_SCORE_HARD, //
};
class LevelCompletionHolder {
public:
	SapphireUUID levelUUID;
	const char* apiName;
};
static const LevelCompletionHolder ACHIEVEMENTS_LEVEL_COMPLETIONS[] { //
{ SapphireUUID { 0x47, 0xd0, 0x1a, 0xe1, 0x20, 0x55, 0xf3, 0x55, 0x14, 0xec, 0x08, 0x15, 0xc2, 0xc6, 0x5d, 0x26, }, "LEVEL_BOMBODROME" }, //
		{ SapphireUUID { 0x31, 0x95, 0x29, 0x34, 0xce, 0xcd, 0x17, 0xd2, 0x80, 0x56, 0x6d, 0xc5, 0x2a, 0x6d, 0xa4, 0x9a, },
				"LEVEL_LIGHTMARE" }, //

};
class DifficultyCompletionHolder {
public:
	const char* apiName;
	SapphireDifficulty difficulty;
	unsigned int playerCount;
	unsigned int targetCount;
};
static const DifficultyCompletionHolder ACHIEVEMENTS_DIFFICULTY_COMPLETIONS[] { //
DifficultyCompletionHolder { "L_TUT1", SapphireDifficulty::Tutorial, 1, 21 }, //
		DifficultyCompletionHolder { "L_SIM1", SapphireDifficulty::Simple, 1, 9 }, //
		DifficultyCompletionHolder { "L_EAS1", SapphireDifficulty::Easy, 1, 26 }, //
		DifficultyCompletionHolder { "L_MOD1", SapphireDifficulty::Moderate, 1, 52 }, //
		DifficultyCompletionHolder { "L_NOR1", SapphireDifficulty::Normal, 1, 91 }, //
		DifficultyCompletionHolder { "L_TRI1", SapphireDifficulty::Tricky, 1, 81 }, //
		DifficultyCompletionHolder { "L_TOU1", SapphireDifficulty::Tough, 1, 69 }, //
		DifficultyCompletionHolder { "L_DIF1", SapphireDifficulty::Difficult, 1, 39 }, //
		DifficultyCompletionHolder { "L_HAR1", SapphireDifficulty::Hard, 1, 37 }, //
		DifficultyCompletionHolder { "L_MAD1", SapphireDifficulty::M_A_D_, 1, 12 }, //

		DifficultyCompletionHolder { "L_TUT2", SapphireDifficulty::Tutorial, 2, 8 }, //
		DifficultyCompletionHolder { "L_SIM2", SapphireDifficulty::Simple, 2, 2 }, //
		DifficultyCompletionHolder { "L_EAS2", SapphireDifficulty::Easy, 2, 7 }, //
		DifficultyCompletionHolder { "L_MOD2", SapphireDifficulty::Moderate, 2, 18 }, //
		DifficultyCompletionHolder { "L_NOR2", SapphireDifficulty::Normal, 2, 41 }, //
		DifficultyCompletionHolder { "L_TRI2", SapphireDifficulty::Tricky, 2, 40 }, //
		DifficultyCompletionHolder { "L_TOU2", SapphireDifficulty::Tough, 2, 12 }, //
		DifficultyCompletionHolder { "L_DIF2", SapphireDifficulty::Difficult, 2, 2 }, //
		DifficultyCompletionHolder { "L_HAR2", SapphireDifficulty::Hard, 2, 1 }, //
		DifficultyCompletionHolder { "L_MAD2", SapphireDifficulty::M_A_D_, 2, 1 }, //
};
class StatisticHolder {
public:
	const char* apiName;
	unsigned int (LevelStatistics::*statMemberPointer)() const;
};
static const StatisticHolder ACHIEVEMENTS_STATISTICS[] { //
StatisticHolder { "C_EMERALD", &LevelStatistics::getEmeraldCollected }, //
		StatisticHolder { "C_CITRINE", &LevelStatistics::getCitrineCollected }, //
		StatisticHolder { "C_SAPPHIRE", &LevelStatistics::getSapphireCollected }, //
		StatisticHolder { "C_RUBY", &LevelStatistics::getRubyCollected }, //
		StatisticHolder { "C_VALUE", &LevelStatistics::getCollectedGemWorth }, //
};
class SkillAchievement: public SapphireSteamAchievement {
	unsigned int threshold;
public:
	SkillAchievement(const char* apiname, unsigned int threshold)
			: SapphireSteamAchievement(apiname), threshold(threshold) {
	}
	virtual unsigned int onSetup(ISteamUserStats* stats, SapphireScene* scene) override {
		return scene->getUserProgressScore() >= threshold ? RESULT_ACHIEVED : RESULT_NONE;
	}
	virtual unsigned int onUserProgressScoreChanged(ISteamUserStats* stats, SapphireScene* scene, unsigned int progress) override {
		return progress >= threshold ? RESULT_ACHIEVED : RESULT_NONE;
	}
};
class LevelCompletionAchievement: public SapphireSteamAchievement {
	const SapphireLevelDescriptor* desc;
public:
	LevelCompletionAchievement(const char* apiname, const SapphireLevelDescriptor* desc)
			: SapphireSteamAchievement(apiname), desc(desc) {
	}
	virtual unsigned int onSetup(ISteamUserStats* stats, SapphireScene* scene) override {
		return desc->state == LevelState::COMPLETED ? RESULT_ACHIEVED : RESULT_NONE;
	}
	virtual unsigned int onLevelStateChanged(ISteamUserStats* stats, SapphireScene* scene, const SapphireLevelDescriptor* descriptor)
			override {
		if (desc != descriptor) {
			return RESULT_NONE;
		}
		if (descriptor->state != LevelState::COMPLETED) {
			return RESULT_NONE;
		}
		return RESULT_ACHIEVED;
	}
};
template<unsigned int Count>
class MultiLevelCompletionAchievement: public SapphireSteamAchievement {
	const SapphireLevelDescriptor* descriptors[Count];

	unsigned int check() {
		for (auto&& d : descriptors) {
			if (d->state != LevelState::COMPLETED) {
				return RESULT_NONE;
			}
		}
		return RESULT_ACHIEVED;
	}
public:
	template<typename ... Args>
	MultiLevelCompletionAchievement(const char* apiname, Args&&... args)
			: SapphireSteamAchievement(apiname), descriptors { args... } {
		static_assert(sizeof...(args) == Count, "Invalid argument count");
	}
	virtual unsigned int onSetup(ISteamUserStats* stats, SapphireScene* scene) override {
		return check();
	}
	virtual unsigned int onLevelStateChanged(ISteamUserStats* stats, SapphireScene* scene, const SapphireLevelDescriptor* descriptor)
			override {
		return check();
	}
};
#define STAT_BUFFER_FOR_ACHIEVEMENT(varname) char varname[256]{"S_"}; memcpy(buffer + 2, getAPIName(), strlen(getAPIName()) + 1)
class DifficultyAchievement: public SapphireSteamAchievement {
	SapphireDifficulty difficulty;
	unsigned int playerCount;
	unsigned int targetCount;

	int32 currentStatValue = 0;

	unsigned int check(ISteamUserStats* stats, SapphireScene* scene) {
		unsigned int finished = 0;
		for (auto it = scene->getDifficultyBegin(playerCount, difficulty), end = scene->getDifficultyEnd(playerCount, difficulty);
				it != end; ++it) {
			if ((*it)->communityLevel) {
				break;
			}
			if ((*it)->state == LevelState::COMPLETED) {
				++finished;
			}
		}
		if (finished != currentStatValue) {
			bool bres;
			if (finished < targetCount && scene->getSettings().steamAchievementProgressIndicatorEnabled) {
				bres = stats->IndicateAchievementProgress(getAPIName(), finished, targetCount);
				ASSERT(bres);
			}
			int32 nval = finished;

			STAT_BUFFER_FOR_ACHIEVEMENT(buffer);
			bres = stats->SetStat(buffer, nval);
			ASSERT(bres);
			if (bres) {
				currentStatValue = finished;

				return finished == targetCount ? RESULT_FINISHED : RESULT_STORE;
			}
		}
		return RESULT_NONE;
	}
public:
	DifficultyAchievement(const char* apiname, SapphireDifficulty difficulty, unsigned int playercount, unsigned int targetcount)
			: SapphireSteamAchievement(apiname), difficulty(difficulty), playerCount(playercount), targetCount(targetcount) {
	}
	virtual unsigned int onSetup(ISteamUserStats* stats, SapphireScene* scene) override {
		STAT_BUFFER_FOR_ACHIEVEMENT(buffer);

		bool res = stats->GetStat(buffer, &currentStatValue);
		ASSERT(res);
		LOGI() << "Stat: " << buffer << ": " << currentStatValue;
		return check(stats, scene);
	}
	virtual unsigned int onLevelStateChanged(ISteamUserStats* stats, SapphireScene* scene, const SapphireLevelDescriptor* descriptor)
			override {
		if (descriptor->state != LevelState::COMPLETED) {
			return RESULT_NONE;
		}
		if (descriptor->difficulty != difficulty) {
			return RESULT_NONE;
		}
		if (descriptor->playerCount != playerCount) {
			return RESULT_NONE;
		}
		return check(stats, scene);
	}
};
class StatisticsAchievement: public SapphireSteamAchievement {
	int32 currentStatValue = 0;

	unsigned int (LevelStatistics::*statMemberPointer)() const;
public:
	StatisticsAchievement(const char* apiname, unsigned int (LevelStatistics::*statmemberpointer)() const)
			: SapphireSteamAchievement(apiname), statMemberPointer(statmemberpointer) {
	}
	virtual unsigned int onSetup(ISteamUserStats* stats, SapphireScene* scene) override {
		STAT_BUFFER_FOR_ACHIEVEMENT(buffer);

		bool res = stats->GetStat(buffer, &currentStatValue);
		ASSERT(res);
		LOGI() << "Stat: " << buffer << ": " << currentStatValue;

		return onTotalStatsChanged(stats, scene, scene->getTotalStatistics());
	}
	virtual unsigned int onTotalStatsChanged(ISteamUserStats* stats, SapphireScene* scene, const LevelStatistics& totalstats) override {
		int32 val = (int32)(totalstats.*statMemberPointer)();
		if (val != currentStatValue) {
			bool bres;

			STAT_BUFFER_FOR_ACHIEVEMENT(buffer);
			bres = stats->SetStat(buffer, val);
			ASSERT(bres);
			if (bres) {
				currentStatValue = val;

				//never finish, as we keep the data updated even after we got the achievement
				return RESULT_STORE;
			}
		}
		return RESULT_NONE;
	}
};
class LevelPlayAchievement: public SapphireSteamAchievement {
protected:
	const SapphireLevelDescriptor* descriptor;

	virtual unsigned int onLevelPLayedImpl(ISteamUserStats* stats, SapphireScene* scene, const Level& level) = 0;
public:
	LevelPlayAchievement(const char* apiname, const SapphireLevelDescriptor* descriptor)
			: SapphireSteamAchievement(apiname), descriptor(descriptor) {
	}

	virtual unsigned int onLevelPLayed(ISteamUserStats* stats, SapphireScene* scene, const SapphireLevelDescriptor* descriptor,
			const Level& level) override {
		if (this->descriptor != descriptor) {
			return RESULT_NONE;
		}
		return onLevelPLayedImpl(stats, scene, level);
	}
};
class DieLevelAchievement: public LevelPlayAchievement {
protected:
	virtual unsigned int onLevelPLayedImpl(ISteamUserStats* stats, SapphireScene* scene, const Level& level) override {
		if (!level.isOver() || level.isSuccessfullyOver()) {
			return RESULT_NONE;
		}
		if (level.getProperties().isMaxTimeConstrained()) {
			if (level.getTurn() > level.getProperties().maxTime) {
				return RESULT_NONE;
			}
		}
		if (level.getProperties().isMaxStepConstrained()) {
			if (level.getMoveCount() > level.getProperties().maxSteps) {
				return RESULT_NONE;
			}
		}
		return RESULT_ACHIEVED;
	}
public:
	DieLevelAchievement(const char* apiname, const SapphireLevelDescriptor* descriptor)
			: LevelPlayAchievement(apiname, descriptor) {
	}

};
void SapphireScene::notifyLevelPlayed(const SapphireLevelDescriptor* desc, const Level& level) {
	if (level.getTurn() == 0) {
		return;
	}
	auto* stats = SteamUserStats();
	if (stats == nullptr) {
		THROW() << "SteamUserStats() is nullptr";
		return;
	}
	iterateAchievementsStore(&SapphireSteamAchievement::onLevelPLayed, stats, this, desc, level);
}
template<typename FunctionType, typename ... Args>
bool SapphireScene::iterateAchievements(FunctionType func, ISteamUserStats* stats, Args&&... args) {
	bool result = false;
	for (unsigned int i = 0; i < steamAchievements.size(); ++i) {
		auto* ach = steamAchievements.get(i);
		unsigned int res = (ach->*func)(stats, util::forward<Args>(args)...);
		switch (res) {
			case SapphireSteamAchievement::RESULT_STORE: {
				result = true;
				break;
			}
			case SapphireSteamAchievement::RESULT_ACHIEVED: {
				bool bres = stats->SetAchievement(ach->getAPIName());
				ASSERT(bres);
			}
				/* no break */
			case SapphireSteamAchievement::RESULT_FINISHED: {
				delete ach;
				steamAchievements.remove(i);
				--i;
				result = true;
				break;
			}
			default: {
				break;
			}
		}
	}
	return result;
}
template<typename FunctionType, typename ... Args>
bool SapphireScene::iterateAchievementsStore(FunctionType func, ISteamUserStats* stats, Args&&... args) {
	if (iterateAchievements<FunctionType, Args...>(func, stats, util::forward<Args>(args)...)) {
		bool res = stats->StoreStats();
		ASSERT(res);
		return true;
	}
	return false;
}
void SapphireScene::prepareSteamAchievements() {
	if (appBorrowed) {
		//do not prepare achievements for borrowed apps
		return;
	}
	auto* stats = SteamUserStats();
	if (stats == nullptr) {
		THROW() << "SteamUserStats() is nullptr";
		return;
	}
	LOGTRACE() << "Preparing steam achievements";
#if RHFW_DEBUG
	stats->ResetAllStats(true);
#endif /* RHFW_DEBUG */
	for (unsigned int i = 0; i < SKILL_ACHIEVEMENTS_COUNT; ++i) {
		bool achieved = false;
		bool res = stats->GetAchievement(ACHIEVEMENTS_SKILL[i], &achieved);
		if (!res || !achieved) {
			steamAchievements.add(new SkillAchievement(ACHIEVEMENTS_SKILL[i], ACHIEVEMENTS_SKILL_THRESHOLDS[i]));
		}
	}

	for (auto&& lch : ACHIEVEMENTS_LEVEL_COMPLETIONS) {
		bool achieved = false;
		bool res = stats->GetAchievement(lch.apiName, &achieved);
		if (!res || !achieved) {
			auto* desc = getLevelWithUUID(lch.levelUUID);
			steamAchievements.add(new LevelCompletionAchievement(lch.apiName, desc));
		}
	}
	for (auto&& dch : ACHIEVEMENTS_DIFFICULTY_COMPLETIONS) {
		bool achieved = false;
		bool res = stats->GetAchievement(dch.apiName, &achieved);
		if (!res || !achieved) {
			steamAchievements.add(new DifficultyAchievement(dch.apiName, dch.difficulty, dch.playerCount, dch.targetCount));
		}
	}
	for (auto&& sh : ACHIEVEMENTS_STATISTICS) {
		steamAchievements.add(new StatisticsAchievement(sh.apiName, sh.statMemberPointer));
	}

	bool achieved = false;
	bool bres;
	bres = stats->GetAchievement("LEVELS_WELTTGAME", &achieved);
	if (!bres || !achieved) {
		steamAchievements.add(new MultiLevelCompletionAchievement<3>("LEVELS_WELTTGAME", //
				getLevelWithUUID(SapphireUUID { 0xa8, 0x68, 0xe9, 0xca, 0xb4, 0xb3, 0x7a, 0x24, 0xd3, 0x16, 0xe7, 0x01, 0x47, 0x77, 0x37,
						0x31 }), //
				getLevelWithUUID(SapphireUUID { 0xc8, 0x25, 0x00, 0x41, 0xf3, 0x53, 0xdb, 0x42, 0xd3, 0xc3, 0x9d, 0x54, 0x6c, 0x88, 0x04,
						0x8d }), //
				getLevelWithUUID(SapphireUUID { 0xe8, 0x9e, 0xd9, 0x1a, 0x7f, 0x6b, 0x91, 0xe6, 0x2c, 0x23, 0x72, 0x3e, 0x7b, 0xf5, 0x87,
						0xaf }) //
						));
	}
	bres = stats->GetAchievement("LEVELS_MISSIMPOSS", &achieved);
	if (!bres || !achieved) {
		steamAchievements.add(new MultiLevelCompletionAchievement<2>("LEVELS_MISSIMPOSS", //
				getLevelWithUUID(SapphireUUID { 0x6b, 0xf2, 0x42, 0x01, 0x55, 0x9e, 0xe4, 0xa9, 0xcf, 0x29, 0x57, 0x46, 0x98, 0x3c, 0xcc,
						0x59, }), //
				getLevelWithUUID(SapphireUUID { 0x30, 0x3e, 0x12, 0x31, 0x0b, 0xa9, 0xc2, 0x16, 0x25, 0x32, 0x63, 0x2c, 0x28, 0x88, 0x0e,
						0x5a, }) //
						));
	}
	bres = stats->GetAchievement("LEVELS_LEO", &achieved);
	if (!bres || !achieved) {
		steamAchievements.add(new MultiLevelCompletionAchievement<5>("LEVELS_LEO", //
				getLevelWithUUID(SapphireUUID { 0x60, 0x36, 0x2b, 0x0b, 0x90, 0x3d, 0xb6, 0x29, 0xe7, 0x6c, 0xda, 0x8d, 0x0b, 0x75, 0x35,
						0xc9, }), //
				getLevelWithUUID(SapphireUUID { 0x16, 0xe2, 0x9c, 0xc3, 0x02, 0xb3, 0x24, 0x48, 0xa7, 0xf1, 0x9e, 0xd4, 0xa4, 0x9e, 0x68,
						0x20, }), //
				getLevelWithUUID(SapphireUUID { 0xca, 0x83, 0xc8, 0x26, 0xfd, 0xb9, 0x13, 0xd6, 0x19, 0xae, 0x42, 0x3d, 0x69, 0xbb, 0x41,
						0x78, }), //
				getLevelWithUUID(SapphireUUID { 0xb9, 0x18, 0xcc, 0x58, 0xd9, 0xfc, 0xd0, 0x38, 0x26, 0xf5, 0xed, 0xc7, 0x2a, 0x03, 0x77,
						0xe0, }), //
				getLevelWithUUID(SapphireUUID { 0xc8, 0x37, 0x29, 0x5d, 0x70, 0xdd, 0x7f, 0xc7, 0xb5, 0x61, 0x8e, 0x90, 0xf4, 0x89, 0xf2,
						0x36, }) //
						));
	}
	bres = stats->GetAchievement("LEVELS_ARMAGEDDON", &achieved);
	if (!bres || !achieved) {
		steamAchievements.add(new MultiLevelCompletionAchievement<2>("LEVELS_ARMAGEDDON", //
				getLevelWithUUID(SapphireUUID { 0x15, 0x89, 0x11, 0x00, 0xde, 0x83, 0xcc, 0x2e, 0xd5, 0xa5, 0x6c, 0x23, 0x13, 0x14, 0x41,
						0x08, }), //
				getLevelWithUUID(SapphireUUID { 0xc3, 0x37, 0x40, 0x8a, 0xe0, 0x03, 0xaf, 0xc4, 0x48, 0x8f, 0x15, 0xa3, 0xaf, 0x35, 0x8b,
						0x72, }) //
						));
	}

	bres = stats->GetAchievement("DIE_THEDOORS", &achieved);
	if (!bres || !achieved) {
		steamAchievements.add(new DieLevelAchievement("DIE_THEDOORS", getLevelWithUUID(SapphireUUID { 0x7a, 0x15, 0xe3, 0xb9, 0x7f, 0xba,
				0x35, 0x31, 0x0b, 0xe1, 0x55, 0x92, 0x0e, 0x1a, 0x3c, 0x39, })));
	}

	iterateAchievementsStore(&SapphireSteamAchievement::onSetup, stats, this);
}
#else
void SapphireScene::notifyLevelPlayed(const SapphireLevelDescriptor* desc, const Level& level) {
	//ignore
}
#endif /* defined(SAPPHIRE_STEAM_API_AVAILABLE) */

LevelStatisticsDialog* SapphireScene::showStatisticsDialogForLevel(SapphireUILayer* parent, const SapphireLevelDescriptor* level,
		const LevelStatistics& stats, const FixedString& steps, uint32 randomseed) {
	LevelStatisticsDialog* statdialog = new LevelStatisticsDialog(parent, level, stats, steps, randomseed);
	statdialog->show(this, true);
	return statdialog;
}
LevelStatisticsDialog* SapphireScene::showStatisticsDialogForLevel(SapphireUILayer* parent, const SapphireLevelDescriptor* level) {
	FixedString steps;
	uint32 randomseed;
	if (getLatestLevelFinishSteps(level, &steps, &randomseed)) {
		Level l;
		if (l.loadLevel(level->getFileDescriptor())) {
			l.setRandomSeed(randomseed);
			DemoPlayer::playMovesUntilSuccess(steps, steps.length() / l.getPlayerCount(), l);
			ASSERT(l.isSuccessfullyOver());
			return showStatisticsDialogForLevel(parent, level, l.getStatistics(), steps, randomseed);
		}
	}
	LevelStatisticsDialog* statdialog = new LevelStatisticsDialog(parent, level);
	statdialog->showDialog(this);
	return statdialog;
}

LeaderboardsDialog* SapphireScene::showLeaderboardsDialogForLevel(SapphireUILayer* parent, const SapphireLevelDescriptor* level) {
	LeaderboardsDialog* dialog = new LeaderboardsDialog(parent, level, level->leaderboards);
	dialog->showDialog(this);
	return dialog;
}

void SapphireScene::onDraw() {
	Scene::onDraw();
}

}  // namespace userapp

