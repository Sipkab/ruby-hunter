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
 * commonmain.cpp
 *
 *  Created on: 2017. okt. 6.
 *      Author: sipka
 */

#include <sapphire/common/commonmain.h>

namespace userapp {

static const char* SapphireMusicStrings[] { //
"A view to a Kill", //
		"Action", //
		"Axel F", //
		"Calm", //
		"Crystal", //
		"Dancing on the Ceiling", //
		"Downunder", //
		"Drive", //
		"FF7choct", //
		"Granite", //
		"Jump", //
		"Live and let Die", //
		"Mission Impossible", //
		"Nightshift", //
		"Only Solutions", //
		"Oxygene4", //
		"Penny Lane", //
		"Pink Panther", //
		"Race", //
		"Ringing", //
		"Staying Alive", //
		"Tico Tico", //
		"Time", //
		"Winner", //
		"Random song", //
};

FixedString convertSapphireMusicToMusicName(SapphireMusic music) {
	if (music < SapphireMusic::_count_of_entries) {
		return SapphireMusicStrings[(unsigned int) music];
	}
	return nullptr;
}

}  // namespace userapp
