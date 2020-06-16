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
 * LevelStatistics.h
 *
 *  Created on: 2017. aug. 4.
 *      Author: sipka
 */

#ifndef JNI_TEST_SAPPHIRE_LEVEL_LEVELSTATISTICS_H_
#define JNI_TEST_SAPPHIRE_LEVEL_LEVELSTATISTICS_H_

#include <framework/io/stream/OutputStream.h>
#include <framework/io/stream/InputStream.h>
#include <gen/types.h>
#include <gen/log.h>
#include <sapphire/sapphireconstants.h>

namespace userapp {
using namespace rhfw;

class LevelStatistics {
public:
	static const unsigned int VERSION = 1;

	unsigned int emeraldCollected = 0;
	unsigned int sapphireCollected = 0;
	unsigned int rubyCollected = 0;
	unsigned int citrineCollected = 0;
	unsigned int dirtMined = 0;
	unsigned int keysCollected = 0;

	unsigned int timeBombsCollected = 0;
	unsigned int timeBombsSet = 0;

	unsigned int sapphiresBroken = 0;
	unsigned int citrinesBroken = 0;

	unsigned int itemsConverted = 0;
	unsigned int lasersFired = 0;
	unsigned int wheelsTurned = 0;

	unsigned int safesOpened = 0;
	unsigned int bagsOpened = 0;

	unsigned int moveCount = 0;
	unsigned int turns = 0;

	unsigned int getCollectedGemWorth() const {
		return emeraldCollected * SAPPHIRE_LOOT_COUNT_EMERALD + citrineCollected * SAPPHIRE_LOOT_COUNT_CITRINE
				+ sapphireCollected * SAPPHIRE_LOOT_COUNT_SAPPHIRE + rubyCollected * SAPPHIRE_LOOT_COUNT_RUBY;
	}
	unsigned int getEmeraldCollected() const {
		return emeraldCollected;
	}
	unsigned int getSapphireCollected() const {
		return sapphireCollected;
	}
	unsigned int getCitrineCollected() const {
		return citrineCollected;
	}
	unsigned int getRubyCollected() const {
		return rubyCollected;
	}

	LevelStatistics& operator+=(const LevelStatistics& o) {
		this->emeraldCollected += o.emeraldCollected;
		this->sapphireCollected += o.sapphireCollected;
		this->rubyCollected += o.rubyCollected;
		this->citrineCollected += o.citrineCollected;
		this->dirtMined += o.dirtMined;
		this->keysCollected += o.keysCollected;

		this->timeBombsCollected += o.timeBombsCollected;
		this->timeBombsSet += o.timeBombsSet;

		this->sapphiresBroken += o.sapphiresBroken;
		this->citrinesBroken += o.citrinesBroken;

		this->itemsConverted += o.itemsConverted;
		this->lasersFired += o.lasersFired;
		this->wheelsTurned += o.wheelsTurned;

		this->safesOpened += o.safesOpened;
		this->bagsOpened += o.bagsOpened;

		this->moveCount += o.moveCount;
		this->turns += o.turns;

		return *this;
	}

	LevelStatistics& operator-=(const LevelStatistics& o) {
		ASSERT(this->emeraldCollected >= o.emeraldCollected);
		ASSERT(this->sapphireCollected >= o.sapphireCollected);
		ASSERT(this->rubyCollected >= o.rubyCollected);
		ASSERT(this->citrineCollected >= o.citrineCollected);
		ASSERT(this->dirtMined >= o.dirtMined);
		ASSERT(this->keysCollected >= o.keysCollected);

		ASSERT(this->timeBombsCollected >= o.timeBombsCollected);
		ASSERT(this->timeBombsSet >= o.timeBombsSet);

		ASSERT(this->sapphiresBroken >= o.sapphiresBroken);
		ASSERT(this->citrinesBroken >= o.citrinesBroken);

		ASSERT(this->itemsConverted >= o.itemsConverted);
		ASSERT(this->lasersFired >= o.lasersFired);
		ASSERT(this->wheelsTurned >= o.wheelsTurned);

		ASSERT(this->safesOpened >= o.safesOpened);
		ASSERT(this->bagsOpened >= o.bagsOpened);

		ASSERT(this->moveCount >= o.moveCount);
		ASSERT(this->turns >= o.turns);

		this->emeraldCollected -= o.emeraldCollected;
		this->sapphireCollected -= o.sapphireCollected;
		this->rubyCollected -= o.rubyCollected;
		this->citrineCollected -= o.citrineCollected;
		this->dirtMined -= o.dirtMined;
		this->keysCollected -= o.keysCollected;

		this->timeBombsCollected -= o.timeBombsCollected;
		this->timeBombsSet -= o.timeBombsSet;

		this->sapphiresBroken -= o.sapphiresBroken;
		this->citrinesBroken -= o.citrinesBroken;

		this->itemsConverted -= o.itemsConverted;
		this->lasersFired -= o.lasersFired;
		this->wheelsTurned -= o.wheelsTurned;

		this->safesOpened -= o.safesOpened;
		this->bagsOpened -= o.bagsOpened;

		this->moveCount -= o.moveCount;
		this->turns -= o.turns;

		return *this;
	}

	LevelStatistics operator-(const LevelStatistics& o) const {
		LevelStatistics result = *this;
		result -= o;
		return result;
	}
	LevelStatistics operator+(const LevelStatistics& o) const {
		LevelStatistics result = *this;
		result += o;
		return result;
	}

	template<unsigned int Version>
	bool serialize(OutputStream& output, unsigned int playcount) const;

	bool serializeSimple(OutputStream& output) const;

	static bool deserialize(InputStream& input, LevelStatistics* outstats, unsigned int* outplaycount) {
		uint32 version;
		auto&& istream = EndianInputStream<Endianness::Big>::wrap(input);
		if (!istream.deserialize<uint32>(version)) {
			return false;
		}
		if (version != VERSION) {
			return false;
		}
		uint32 playcount;
		if (!istream.deserialize<uint32>(playcount) || !istream.deserialize<uint32>(outstats->emeraldCollected)
				|| !istream.deserialize<uint32>(outstats->sapphireCollected) || !istream.deserialize<uint32>(outstats->rubyCollected)
				|| !istream.deserialize<uint32>(outstats->citrineCollected) || !istream.deserialize<uint32>(outstats->dirtMined)
				|| !istream.deserialize<uint32>(outstats->keysCollected) || !istream.deserialize<uint32>(outstats->timeBombsCollected)
				|| !istream.deserialize<uint32>(outstats->timeBombsSet) || !istream.deserialize<uint32>(outstats->sapphiresBroken)
				|| !istream.deserialize<uint32>(outstats->citrinesBroken) || !istream.deserialize<uint32>(outstats->itemsConverted)
				|| !istream.deserialize<uint32>(outstats->lasersFired) || !istream.deserialize<uint32>(outstats->wheelsTurned)
				|| !istream.deserialize<uint32>(outstats->safesOpened) || !istream.deserialize<uint32>(outstats->bagsOpened)
				|| !istream.deserialize<uint32>(outstats->moveCount) || !istream.deserialize<uint32>(outstats->turns)) {
			return false;
		}
		*outplaycount = playcount;
		return true;
	}
};

}  // namespace userapp

#endif /* JNI_TEST_SAPPHIRE_LEVEL_LEVELSTATISTICS_H_ */
