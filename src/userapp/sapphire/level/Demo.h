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
 * Demo.h
 *
 *  Created on: 2016. jan. 17.
 *      Author: sipka
 */

#ifndef DEMO_H_
#define DEMO_H_

#include <framework/utils/FixedString.h>
#include <framework/utils/utility.h>
#include <gen/types.h>
#include <sapphire/level/DemoInfo.h>

namespace userapp {
using namespace rhfw;

class Demo {
public:
	DemoInfo info;

	FixedString moves;

	unsigned int randomseed = 0;

	bool userDemo = false;

	Demo() {
	}

	Demo(DemoInfo info, FixedString moves, unsigned int randomseed, bool userdemo = false)
			: info(util::move(info)), moves(util::move(moves)), randomseed(randomseed), userDemo(userdemo) {
	}

};
class Level;

class DemoPlayer {
	const char* moves = nullptr;
	unsigned int moveCount = 0;
	unsigned int index = 0;

	static void applyCommandForPlayer(Level& level, unsigned int player, char command);

public:
	static char encodeDemoMove(SapphireDirection dir, SapphireControl control);
	static void decodeDemoMove(char move, SapphireDirection* dir, SapphireControl* control);

	static void playMoves(const char* moves, unsigned int count, Level& level);
	static void playMovesUntilSuccess(const char* moves, unsigned int count, Level& level);

	void play(const Demo* demo, Level& level);
	void play(const char* moves, unsigned int movecount);
	void play(const Demo* demo, Level& level, unsigned int turns);
	void next(Level& level);
	void next(Level& level, unsigned int turns);
	void stop();

	void applyTurnForPlayer(Level& level, unsigned int player);

	void playFully(const Demo* demo, Level& level);

	bool verifySuccess(const Demo* demo, Level& level);

	bool isOver() const {
		return moves == nullptr || index >= moveCount;
	}
	bool isPlaying() const {
		return !isOver();
	}

	void setIndex(unsigned int index) {
		this->index = index;
	}

	unsigned int getCurrentIndex() const {
		return index;
	}
	unsigned int getStepCount() const {
		return moveCount;
	}

	SapphireControl getControl(unsigned int index) const {
		switch (moves[index]) {
			case 'l':
			case 't':
			case 'r':
			case 'b': {
				return SapphireControl::Move;
			}
			case 'L':
			case 'T':
			case 'R':
			case 'B': {
				return SapphireControl::Take;
			}
			case 'm':
			case 'u':
			case 's':
			case 'c': {
				return SapphireControl::PutBomb;
			}
			default: {
				return SapphireControl::Undefined;
			}
		}
	}
	SapphireDirection getDirection(unsigned int index) const {
		switch (moves[index]) {
			case 'l':
			case 'L':
			case 'm': {
				return SapphireDirection::Left;
			}
			case 't':
			case 'T':
			case 'u': {
				return SapphireDirection::Up;
			}
			case 'r':
			case 'R':
			case 's': {
				return SapphireDirection::Right;
			}
			case 'b':
			case 'B':
			case 'c': {
				return SapphireDirection::Down;
			}
			default: {
				return SapphireDirection::Undefined;
			}
		}
	}
};

}

#endif /* DEMO_H_ */
