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
 * SapphireSteamAchievement.h
 *
 *  Created on: 2017. szept. 6.
 *      Author: sipka
 */

#ifndef JNI_TEST_SAPPHIRE_SAPPHIRESTEAMACHIEVEMENT_H_
#define JNI_TEST_SAPPHIRE_SAPPHIRESTEAMACHIEVEMENT_H_

#include <sapphire/steam_opt.h>

namespace userapp {
class SapphireScene;
class SapphireLevelDescriptor;
class Level;

class SapphireSteamAchievement {
private:
	const char* apiName;
protected:

public:
	static const unsigned int RESULT_NONE = 0x00;
	static const unsigned int RESULT_STORE = 0x01;
	static const unsigned int RESULT_ACHIEVED = 0x02;
	static const unsigned int RESULT_FINISHED = 0x03;
	SapphireSteamAchievement(const char* apiname)
			: apiName(apiname) {
	}
	virtual ~SapphireSteamAchievement() = default;

	virtual unsigned int onTotalStatsChanged(ISteamUserStats* stats, SapphireScene* scene, const LevelStatistics& totalstats) {
		return RESULT_NONE;
	}

	virtual unsigned int onLevelStateChanged(ISteamUserStats* stats, SapphireScene* scene, const SapphireLevelDescriptor* descriptor) {
		return RESULT_NONE;
	}

	virtual unsigned int onUserProgressScoreChanged(ISteamUserStats* stats, SapphireScene* scene, unsigned int progress) {
		return RESULT_NONE;
	}

	virtual unsigned int onLevelPLayed(ISteamUserStats* stats, SapphireScene* scene, const SapphireLevelDescriptor* descriptor,
			const Level& level) {
		return RESULT_NONE;
	}

	virtual unsigned int onSetup(ISteamUserStats* stats, SapphireScene* scene) {
		return RESULT_NONE;
	}

	const char* getAPIName() const {
		return apiName;
	}
};

}  // namespace userapp

#endif /* JNI_TEST_SAPPHIRE_SAPPHIRESTEAMACHIEVEMENT_H_ */
