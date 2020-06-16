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
 * Demo.cpp
 *
 *  Created on: 2016. jan. 17.
 *      Author: sipka
 */

#include <sapphire/level/Demo.h>
#include <sapphire/level/Level.h>

namespace userapp {

char DemoPlayer::encodeDemoMove(SapphireDirection dir, SapphireControl control) {
	switch (control) {
		case SapphireControl::Undefined: {
			return '.';
		}
		case SapphireControl::Move: {
			switch (dir) {
				case SapphireDirection::Left: {
					return 'l';
				}
				case SapphireDirection::Up: {
					return 't';
				}
				case SapphireDirection::Right: {
					return 'r';
				}
				case SapphireDirection::Down: {
					return 'b';
				}
				default: {
					THROW()<< "Invalid control direction: " << dir;
					return '.';
				}
			}
		}
		case SapphireControl::Take: {
			switch (dir) {
				case SapphireDirection::Left: {
					return 'L';
				}
				case SapphireDirection::Up: {
					return 'T';
				}
				case SapphireDirection::Right: {
					return 'R';
				}
				case SapphireDirection::Down: {
					return 'B';
				}
				default: {
					THROW() << "Invalid control direction: " << dir;
					return '.';
				}
			}
		}
		case SapphireControl::PutBomb: {
			switch (dir) {
				case SapphireDirection::Left: {
					return 'm';
				}
				case SapphireDirection::Up: {
					return 'u';
				}
				case SapphireDirection::Right: {
					return 's';
				}
				case SapphireDirection::Down: {
					return 'c';
				}
				default: {
					THROW() << "Invalid control direction: " << dir;
					return '.';
				}
			}
		}
		default: {
			THROW() << "Invalid control: " << control;
			return '.';
		}
	}
}
void DemoPlayer::decodeDemoMove(char move, SapphireDirection* dir, SapphireControl* control) {
	switch (move) {
		case 'r':
			*dir = SapphireDirection::Right;
			*control = SapphireControl::Move;
			break;
		case 'l':
			*dir = SapphireDirection::Left;
			*control = SapphireControl::Move;
			break;
		case 'b':
			*dir = SapphireDirection::Down;
			*control = SapphireControl::Move;
			break;
		case 't':
			*dir = SapphireDirection::Up;
			*control = SapphireControl::Move;
			break;
		case 'R':
			*dir = SapphireDirection::Right;
			*control = SapphireControl::Take;
			break;
		case 'L':
			*dir = SapphireDirection::Left;
			*control = SapphireControl::Take;
			break;
		case 'B':
			*dir = SapphireDirection::Down;
			*control = SapphireControl::Take;
			break;
		case 'T':
			*dir = SapphireDirection::Up;
			*control = SapphireControl::Take;
			break;
		case 's': //bombright
			*dir = SapphireDirection::Right;
			*control = SapphireControl::PutBomb;
			break;
		case 'c': //bombdown
			*dir = SapphireDirection::Down;
			*control = SapphireControl::PutBomb;
			break;
		case 'm': //bombleft
			*dir = SapphireDirection::Left;
			*control = SapphireControl::PutBomb;
			break;
		case 'u': //bombup
			*dir = SapphireDirection::Up;
			*control = SapphireControl::PutBomb;
			break;
		case '.':
			*dir = SapphireDirection::Undefined;
			*control = SapphireControl::Undefined;
			break;
		default: {
			THROW()<< "unknown demo command: " << (unsigned int) move;
			//failsafe
			*dir = SapphireDirection::Undefined;
			*control = SapphireControl::Undefined;
			break;
		}
	}
}
void DemoPlayer::applyCommandForPlayer(Level& level, unsigned int player, char command) {
	//LOGV("Demo turn for player: %u: %c", player, command);
	switch (command) {
		case 'r':
			level.applyControl(player, SapphireDirection::Right, SapphireControl::Move);
			break;
		case 'l':
			level.applyControl(player, SapphireDirection::Left, SapphireControl::Move);
			break;
		case 'b':
			level.applyControl(player, SapphireDirection::Down, SapphireControl::Move);
			break;
		case 't':
			level.applyControl(player, SapphireDirection::Up, SapphireControl::Move);
			break;
		case 'R':
			level.applyControl(player, SapphireDirection::Right, SapphireControl::Take);
			break;
		case 'L':
			level.applyControl(player, SapphireDirection::Left, SapphireControl::Take);
			break;
		case 'B':
			level.applyControl(player, SapphireDirection::Down, SapphireControl::Take);
			break;
		case 'T':
			level.applyControl(player, SapphireDirection::Up, SapphireControl::Take);
			break;
		case 's': //bombright
			level.applyControl(player, SapphireDirection::Right, SapphireControl::PutBomb);
			break;
		case 'c': //bombdown
			level.applyControl(player, SapphireDirection::Down, SapphireControl::PutBomb);
			break;
		case 'm': //bombleft
			level.applyControl(player, SapphireDirection::Left, SapphireControl::PutBomb);
			break;
		case 'u': //bombup
			level.applyControl(player, SapphireDirection::Up, SapphireControl::PutBomb);
			break;
		case '.':
			break;
		default: {
			THROW()<< "unknown demo command: " << (unsigned int) command;
			break;
		}
	}
}
void DemoPlayer::applyTurnForPlayer(Level& level, unsigned int player) {
	ASSERT(index < moveCount) << "Out of bounds: [" << index << "] for: " << moveCount;
	char command = moves[index++];
	applyCommandForPlayer(level, player, command);
}

void DemoPlayer::next(Level& level) {
	if (isOver())
		return;

	unsigned int pcount = level.getPlayerCount();
	for (unsigned int i = 0; i < pcount; ++i) {
		applyTurnForPlayer(level, i);
	}
	level.applyTurn();
}
void DemoPlayer::next(Level& level, unsigned int turns) {
	ASSERT(index + turns <= moveCount);
	unsigned int pcount = level.getPlayerCount();
	while (turns > 0) {
		for (unsigned int i = 0; i < pcount; ++i) {
			applyTurnForPlayer(level, i);
		}
		level.applyTurn();
		--turns;
	}
}

void DemoPlayer::play(const Demo* demo, Level& level) {
	this->index = 0;
	this->moves = demo->moves;
	this->moveCount = demo->moves.length();
	level.setRandomSeed(demo->randomseed);
}
void DemoPlayer::play(const char* moves, unsigned int movecount) {
	this->index = 0;
	this->moves = moves;
	this->moveCount = movecount;
}
void DemoPlayer::play(const Demo* demo, Level& level, unsigned int turns) {
	play(demo, level);
	next(level, turns);
}

void DemoPlayer::stop() {
	this->index = 0xFFFFFFFF;
}

void DemoPlayer::playFully(const Demo* demo, Level& level) {
	unsigned int pcount = level.getPlayerCount();
	play(demo, level);
	while (!level.isOver() && isPlaying()) {
		for (unsigned int i = 0; i < pcount; ++i) {
			applyTurnForPlayer(level, i);
		}
		level.applyTurn();
	}
}

void DemoPlayer::playMoves(const char* moves, unsigned int turncount, Level& level) {
	unsigned int pcount = level.getPlayerCount();
	for (unsigned int i = 0; i < turncount; ++i) {
		for (int p = 0; p < pcount; ++p) {
			applyCommandForPlayer(level, p, moves[i * pcount + p]);
		}
		level.applyTurn();
	}
}
void DemoPlayer::playMovesUntilSuccess(const char* moves, unsigned int turncount, Level& level) {
	unsigned int pcount = level.getPlayerCount();
	for (unsigned int i = 0; i < turncount; ++i) {
		for (int p = 0; p < pcount; ++p) {
			applyCommandForPlayer(level, p, moves[i * pcount + p]);
		}
		level.applyTurn();
		if (level.isSuccessfullyOver()) {
			break;
		}
	}
}

bool DemoPlayer::verifySuccess(const Demo* demo, Level& level) {
	play(demo, level);
	while (!level.isOver() && isPlaying()) {
		unsigned int pcount = level.getPlayerCount();
		for (unsigned int i = 0; i < pcount; ++i) {
			applyTurnForPlayer(level, i);
		}
		level.applyTurn();
	}
	return level.isSuccessfullyOver();
}

} // namespace userapp

