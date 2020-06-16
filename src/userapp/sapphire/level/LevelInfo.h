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
 * LevelInfo.h
 *
 *  Created on: 2016. jan. 17.
 *      Author: sipka
 */

#ifndef LEVELINFO_H_
#define LEVELINFO_H_

#include <framework/utils/FixedString.h>
#include <sapphire/level/SapphireUUID.h>
#include <sapphire/level/LevelAuthor.h>

#include <gen/configuration.h>
#include <gen/types.h>
#include <gen/log.h>

namespace userapp {
using namespace rhfw;
class LevelInfo {
public:
	FixedString title;
	LevelAuthor author;
	FixedString description;
	SapphireLevelCategory category = SapphireLevelCategory::None;
	SapphireDifficulty difficulty = SapphireDifficulty::Unrated;
	SapphireUUID uuid;
	bool nonModifyAbleFlag = false;
};

}

#endif /* LEVELINFO_H_ */
