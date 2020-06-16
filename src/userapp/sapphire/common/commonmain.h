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
 * commonmain.h
 *
 *  Created on: 2017. okt. 6.
 *      Author: sipka
 */

#ifndef JNI_TEST_SAPPHIRE_COMMON_COMMONMAIN_H_
#define JNI_TEST_SAPPHIRE_COMMON_COMMONMAIN_H_

#include <framework/utils/FixedString.h>
#include <gen/fwd/types.h>

namespace userapp {

using namespace rhfw;

enum class SapphireMusic
	: uint32 {
		a_view_to_a_kill = 0,
	action = 1,
	axel_f = 2,
	calm = 3,
	crystal = 4,
	dancing_on_the_ceiling = 5,
	downunder = 6,
	drive = 7,
	ff7choct = 8,
	granite = 9,
	jump = 10,
	live_and_let_die = 11,
	mission_impossible = 12,
	nightshift = 13,
	only_solutions = 14,
	oxygene4 = 15,
	penny_lane = 16,
	pink_panther = 17,
	race = 18,
	ringing = 19,
	staying_alive = 20,
	tico_tico = 21,
	time = 22,
	winner = 23,
	_count_of_entries = 24,
};

FixedString convertSapphireMusicToMusicName(SapphireMusic music);

}  // namespace userapp

#endif /* JNI_TEST_SAPPHIRE_COMMON_COMMONMAIN_H_ */
