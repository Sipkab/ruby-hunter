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
 * sapphireconstants.h
 *
 *  Created on: 2016. okt. 11.
 *      Author: sipka
 */

#ifndef TEST_SAPPHIRE_SAPPHIRECONSTANTS_H_
#define TEST_SAPPHIRE_SAPPHIRECONSTANTS_H_

#include <gen/types.h>
#include <gen/serialize.h>
#include <gen/platform.h>

#define SAPPHIRE_COLOR Color { 0, 0.375, 1, 1 }
#define SAPPHIRE_GRAYED_COLOR Color { 0.5f, 0.5f, 0.5f, 1 }
#define SAPPHIRE_TITLE_PERCENT (1.0f/6.0f)
#define SAPPHIRE_TITLE_PADDING_PERCENT 1.2f
#define SAPPHIRE_DIFFICULTY_COUNT ((int) SapphireDifficulty::_count_of_entries)
#define SAPPHIRE_TURN_FAST_MILLIS 50
#define SAPPHIRE_TURN_INCREASE_STEP_MILLIS 50
#define SAPPHIRE_TURN_MILLIS 250
#define SAPPHIRE_TURN_SLOW_MILLIS 500

#define SAPPHIRE_CMD_END_OF_FILE ((char)0)
#define SAPPHIRE_CMD_DEMOCOUNT ((char)128)
#define SAPPHIRE_CMD_PLAYERCOUNT ((char)129)
#define SAPPHIRE_CMD_UUID ((char)130)
#define SAPPHIRE_CMD_LEADERBOARDS ((char)131)
#define SAPPHIRE_CMD_NON_MODIFYABLE_FLAG ((char)'X')

#define SAPPHIRE_LEVEL_AUTHOR_MAX_LEN 63
#define SAPPHIRE_LEVEL_DESCRIPTION_MAX_LEN 1023
#define SAPPHIRE_LEVEL_TITLE_MAX_LEN 63
#define SAPPHIRE_DEMO_MAX_LEN (1024 * 1024)
#define SAPPHIRE_DISCUSSION_MESSAGE_MAX_LEN 127
#define SAPPHIRE_REPORT_REASON_MAX_LEN 127
#define SAPPHIRE_USERNAME_MAX_LEN 31
#define SAPPHIRE_USERNAME_MIN_LEN 4
#define SAPPHIRE_DEMO_NAME_MAX_LEN SAPPHIRE_USERNAME_MAX_LEN
#define SAPPHIRE_FEEDBACK_MAX_LEN SAPPHIRE_DISCUSSION_MESSAGE_MAX_LEN
#define SAPPHIRE_MUSIC_NAME_MAX_LEN 127

#define SAPPHIRE_MUSIC_DIRECTORY_NAME "music"

#define SAPPHIRE_LINK_NUMBER_LENGTH 6
#define SAPPHIRE_LINK_NUMBER_MINIMUM 111111
#define SAPPHIRE_LINK_NUMBER_MAXIMUM 999999

#define SAPPHIRE_LOOT_COUNT_EMERALD		1
#define SAPPHIRE_LOOT_COUNT_CITRINE		2
#define SAPPHIRE_LOOT_COUNT_SAPPHIRE	3
#define SAPPHIRE_LOOT_COUNT_RUBY		5

#define SAPPHIRE_PLAYER_PLACEHOLDER_NAME "Player"

#define SAPPHIRE_DIFFICULTY_SCORE_TUTORIAL	20
#define SAPPHIRE_DIFFICULTY_SCORE_SIMPLE	50
#define SAPPHIRE_DIFFICULTY_SCORE_EASY		140
#define SAPPHIRE_DIFFICULTY_SCORE_MODERATE	300
#define SAPPHIRE_DIFFICULTY_SCORE_NORMAL	600
#define SAPPHIRE_DIFFICULTY_SCORE_TRICKY	1100
#define SAPPHIRE_DIFFICULTY_SCORE_TOUGH		1950
#define SAPPHIRE_DIFFICULTY_SCORE_DIFFICULT	3200
#define SAPPHIRE_DIFFICULTY_SCORE_HARD	4800
#define SAPPHIRE_DIFFICULTY_SCORE_M_A_D_

#define SAPPHIRE_GAME_OLD_NAME "Sapphire Yours"
#define SAPPHIRE_GAME_NAME "Ruby Hunter"

//starting at zero
//0: android-ios-winstore release
//1: 3d & level editor
//2: desktop versions & community hub
//3: minor adjustments as suggested by @ppp
//4: fix for enum dialog item listener
//5: steam release, ruby hunter
//6: speed increase/decrease keys aded
#define SAPPHIRE_RELEASE_VERSION_NUMBER 6
#define SAPPHIRE_LEVEL_VERSION_NUMBER 5
/*
 * !!!update server when increasing release number!!!
 */

#define SAPPHIRE_MAX_LEVEL_DIMENSION 512
#define SAPPHIRE_MAX_YAMYAMREMAINDER_COUNT 512

#define SAPPHIRE_SERVER_HOSTNAME "rh-server.sipka.dev"
#define SAPPHIRE_SERVER_PORT_NUMBER 41030
#define SAPPHIRE_SERVER_CONTROL_PORT_NUMBER 15249

#define SAPPHIRE_SERVER_HELLO_STRING "Hello Sapphire!"
#define SAPPHIRE_CLIENT_HELLO_STRING "Hello Yours!"

#define SAPPHIRE_LEVEL3D_DEPTH_THRESHOLD 0.75f
#define SAPPHIRE_BACKGROUND_DIFFUSE_GRAY_COLOR 0.14f

#define SAPPHIREMUSIC_UNSPECIFIED ((SapphireMusic)0xFFFFFFFF)

//put // at the start of the next line to turn on screenshot mode
/*
 #define SAPPHIRE_SCREENSHOT_MODE 1
 #define SAPPHIRE_SCREENSHOT_INCLUDE_HUD 1
 //*/

typedef rhfw::uint16 SY_COMM_CMD;
namespace rhfw {

template<Endianness ENDIAN>
class SerializeExecutor<SapphireComm, ENDIAN> {
public:
	template<typename InStream>
	static bool deserialize(InStream& is, SapphireComm& outdata) {
		bool res = SerializeExecutor<SY_COMM_CMD, ENDIAN>::deserialize(is, reinterpret_cast<SY_COMM_CMD&>(outdata));
		if (res) {
			if ((unsigned int) outdata < (unsigned int) SapphireComm::MIN) {
				return false;
			}
			if ((unsigned int) outdata > (unsigned int) SapphireComm::MAX) {
				return false;
			}
		}
		return res;
	}

	template<typename OutStream>
	static bool serialize(OutStream& os, const SapphireComm& data) {
		return SerializeExecutor<SY_COMM_CMD, ENDIAN>::serialize(os, reinterpret_cast<const SY_COMM_CMD&>(data));
	}
};
template<Endianness ENDIAN>
class SerializeExecutor<SapphireCommError, ENDIAN> {
public:
	template<typename InStream>
	static bool deserialize(InStream& is, SapphireCommError& outdata) {
		bool res = SerializeExecutor<SY_COMM_CMD, ENDIAN>::deserialize(is, reinterpret_cast<SY_COMM_CMD&>(outdata));
		if (res) {
			if ((unsigned int) outdata >= (unsigned int) SapphireCommError::_count_of_entries) {
				return false;
			}
		}
		return res;
	}

	template<typename OutStream>
	static bool serialize(OutStream& os, const SapphireCommError& data) {
		return SerializeExecutor<SY_COMM_CMD, ENDIAN>::serialize(os, reinterpret_cast<const SY_COMM_CMD&>(data));
	}
};

template<Endianness ENDIAN>
class SerializeExecutor<SapphireCommunityInformation, ENDIAN> {
public:
	template<typename InStream>
	static bool deserialize(InStream& is, SapphireCommunityInformation& outdata) {
		bool res = SerializeExecutor<SY_COMM_CMD, ENDIAN>::deserialize(is, reinterpret_cast<SY_COMM_CMD&>(outdata));
		if (res) {
			if ((unsigned int) outdata >= (unsigned int) SapphireCommunityInformation::_count_of_entries) {
				return false;
			}
		}
		return res;
	}

	template<typename OutStream>
	static bool serialize(OutStream& os, const SapphireCommunityInformation& data) {
		return SerializeExecutor<SY_COMM_CMD, ENDIAN>::serialize(os, reinterpret_cast<const SY_COMM_CMD&>(data));
	}
};
template<Endianness ENDIAN>
class SerializeExecutor<SapphireLevelCommProgress, ENDIAN> {
public:
	template<typename InStream>
	static bool deserialize(InStream& is, SapphireLevelCommProgress& outdata) {
		bool res = SerializeExecutor<SY_COMM_CMD, ENDIAN>::deserialize(is, reinterpret_cast<SY_COMM_CMD&>(outdata));
		if (res) {
			if ((unsigned int) outdata >= (unsigned int) SapphireLevelCommProgress::COMM_COUNT) {
				return false;
			}
		}
		return res;
	}

	template<typename OutStream>
	static bool serialize(OutStream& os, const SapphireLevelCommProgress& data) {
		return SerializeExecutor<SY_COMM_CMD, ENDIAN>::serialize(os, reinterpret_cast<const SY_COMM_CMD&>(data));
	}
};

template<Endianness ENDIAN>
class SerializeExecutor<SapphireDifficulty, ENDIAN> {
public:
	template<typename InStream>
	static bool deserialize(InStream& is, SapphireDifficulty& outdata) {
		bool res = SerializeExecutor<uint32, ENDIAN>::deserialize(is, reinterpret_cast<uint32&>(outdata));
		if (res) {
			if ((unsigned int) outdata >= (unsigned int) SapphireDifficulty::_count_of_entries) {
				return false;
			}
		}
		return res;
	}

	template<typename OutStream>
	static bool serialize(OutStream& os, const SapphireDifficulty& data) {
		return SerializeExecutor<uint32, ENDIAN>::serialize(os, reinterpret_cast<const uint32&>(data));
	}
};
template<Endianness ENDIAN>
class SerializeExecutor<SapphireLevelCategory, ENDIAN> {
public:
	template<typename InStream>
	static bool deserialize(InStream& is, SapphireLevelCategory& outdata) {
		bool res = SerializeExecutor<uint32, ENDIAN>::deserialize(is, reinterpret_cast<uint32&>(outdata));
		if (res) {
			if ((unsigned int) outdata >= (unsigned int) SapphireLevelCategory::_count_of_entries) {
				return false;
			}
		}
		return res;
	}

	template<typename OutStream>
	static bool serialize(OutStream& os, const SapphireLevelCategory& data) {
		return SerializeExecutor<uint32, ENDIAN>::serialize(os, reinterpret_cast<const uint32&>(data));
	}
};

template<Endianness ENDIAN>
class SerializeExecutor<SapphireLeaderboards, ENDIAN> {
public:
	template<typename InStream>
	static bool deserialize(InStream& is, SapphireLeaderboards& outdata) {
		bool res = SerializeExecutor<uint32, ENDIAN>::deserialize(is, reinterpret_cast<uint32&>(outdata));
		if (res) {
			if ((outdata & ~SapphireLeaderboards::UsedFlags) != 0) {
				return false;
			}
		}
		return res;
	}

	template<typename OutStream>
	static bool serialize(OutStream& os, const SapphireLeaderboards& data) {
		return SerializeExecutor<uint32, ENDIAN>::serialize(os, reinterpret_cast<const uint32&>(data));
	}
};

}  // namespace rhfw

namespace userapp {
extern const rhfw::uint8 SAPPHIRE_COMM_PV_KEY[256];
}  // namespace userapp

#ifndef RHFW_HARDWARE_KEYBOARD_EVENTS_UNSUPPORTED
#if defined(RHFW_PLATFORM_MACOSX) || defined(RHFW_PLATFORM_WIN32) || defined(RHFW_PLATFORM_LINUX)
#define SAPPHIRE_DUAL_PLAYER_AVAILABLE 1
#define SAPPHIRE_NO_ONSCREEN_CONTROLS 1
#define SAPPHIRE_MAX_PLAYER_COUNT 2
#else
#define SAPPHIRE_MAX_PLAYER_COUNT 1
#endif /* defined(RHFW_PLATFORM_MACOSX) || defined(RHFW_PLATFORM_WIN32) || defined(RHFW_PLATFORM_LINUX) */
#endif /* !defined(RHFW_HARDWARE_KEYBOARD_EVENTS_UNSUPPORTED) */

#if !defined(SAPPHIRE_MAX_PLAYER_COUNT)
#define SAPPHIRE_MAX_PLAYER_COUNT 1
#endif /* !defined(SAPPHIRE_MAX_PLAYER_COUNT) */

#if defined(SAPPHIRE_STEAM_API_AVAILABLE) && (defined(RHFW_PLATFORM_WIN32) || defined(RHFW_PLATFORM_MACOSX) || defined(RHFW_PLATFORM_LINUX))
#define SAPPHIRE_STEAM_MAIN_APP_ID 675870
#define SAPPHIRE_STEAM_APP_ID SAPPHIRE_STEAM_MAIN_APP_ID

#endif /* defined(SAPPHIRE_STEAM_API_AVAILABLE) */

#endif /* TEST_SAPPHIRE_SAPPHIRECONSTANTS_H_ */
