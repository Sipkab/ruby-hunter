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
 * LevelAuthor.h
 *
 *  Created on: 2016. okt. 8.
 *      Author: sipka
 */

#ifndef TEST_SAPPHIRE_LEVEL_LEVELAUTHOR_H_
#define TEST_SAPPHIRE_LEVEL_LEVELAUTHOR_H_

#include <framework/utils/FixedString.h>
#include <sapphire/level/SapphireUUID.h>
#include <sapphire/community/SapphireUser.h>

namespace userapp {
using namespace rhfw;

class LevelAuthor {
	FixedString userName;
public:

	LevelAuthor& operator=(const SapphireUser& user) {
		this->userName = user.getUserName();
		return *this;
	}
	LevelAuthor& operator=(const FixedString& name) {
		this->userName = name;
		return *this;
	}

	const FixedString& getUserName() const {
		return userName;
	}
	FixedString& getUserName() {
		return userName;
	}
};

} // namespace userapp

#endif /* TEST_SAPPHIRE_LEVEL_LEVELAUTHOR_H_ */
