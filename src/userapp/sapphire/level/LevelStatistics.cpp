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
 * LevelStatistics.cpp
 *
 *  Created on: 2017. aug. 4.
 *      Author: sipka
 */

#include <sapphire/level/LevelStatistics.h>

namespace userapp {
using namespace rhfw;

template<>
bool LevelStatistics::serialize<1>(OutputStream& output, unsigned int playcount) const {
	auto&& ostream = EndianOutputStream<Endianness::Big>::wrap(output);
	bool res = true;
	res = res && ostream.serialize<uint32>(1);
	res = res && ostream.serialize<uint32>(playcount);

	res = res && ostream.serialize<uint32>(emeraldCollected);
	res = res && ostream.serialize<uint32>(sapphireCollected);
	res = res && ostream.serialize<uint32>(rubyCollected);
	res = res && ostream.serialize<uint32>(citrineCollected);
	res = res && ostream.serialize<uint32>(dirtMined);
	res = res && ostream.serialize<uint32>(keysCollected);

	res = res && ostream.serialize<uint32>(timeBombsCollected);
	res = res && ostream.serialize<uint32>(timeBombsSet);

	res = res && ostream.serialize<uint32>(sapphiresBroken);
	res = res && ostream.serialize<uint32>(citrinesBroken);

	res = res && ostream.serialize<uint32>(itemsConverted);
	res = res && ostream.serialize<uint32>(lasersFired);
	res = res && ostream.serialize<uint32>(wheelsTurned);

	res = res && ostream.serialize<uint32>(safesOpened);
	res = res && ostream.serialize<uint32>(bagsOpened);

	res = res && ostream.serialize<uint32>(moveCount);
	res = res && ostream.serialize<uint32>(turns);
	return res;
}

bool userapp::LevelStatistics::serializeSimple(OutputStream& output) const {
	return serialize<VERSION>(output, 1);
}

}  // namespace userapp

