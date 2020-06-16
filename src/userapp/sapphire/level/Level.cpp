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
 * Level.cpp
 *
 *  Created on: 2016. jan. 15.
 *      Author: sipka
 */

#include <framework/io/files/AssetFileDescriptor.h>
#include <framework/io/stream/InputStream.h>

#include <sapphire/level/Level.h>
#include <sapphire/level/SapphireUUID.h>
#include <sapphire/sapphireconstants.h>
#include <sapphire/common/commonmain.h>

#include <gen/log.h>
#include <gen/configuration.h>

#include <stdlib.h>

namespace userapp {

class GameObjectMap {
public:

	Level::GameObject sample[(unsigned int) SapphireObject::_count_of_entries];

	const Level::GameObject& operator [](SapphireObject o) {
		return sample[(unsigned int) o];
	}

#define SET_SAMPLE(obj, flag) sample[(unsigned int)SapphireObject::obj].object = SapphireObject::obj; sample[(unsigned int)SapphireObject::obj].props = flag;

	GameObjectMap() {
		SET_SAMPLE(Player, SapphireProps::Blowable);
		SET_SAMPLE(Air, SapphireProps::Blowable | SapphireProps::Pickable);
		SET_SAMPLE(Bag,
				SapphireProps::Fallable | SapphireProps::Blowable | SapphireProps::Round | SapphireProps::Pushable
						| SapphireProps::ObjectPushable | SapphireProps::Convertable);
		SET_SAMPLE(Bomb,
				SapphireProps::Fallable | SapphireProps::Blowable | SapphireProps::Round | SapphireProps::Pushable
						| SapphireProps::ObjectPushable | SapphireProps::ExplodePropagate);
		SET_SAMPLE(Bug, SapphireProps::Blowable | SapphireProps::Enemy | SapphireProps::ExplodePropagate);
		SET_SAMPLE(Converter, SapphireProps::Blowable);
		SET_SAMPLE(Citrine,
				SapphireProps::Fallable | SapphireProps::Round | SapphireProps::Convertable | SapphireProps::Blowable
						| SapphireProps::ObjectPushable | SapphireProps::Pickable);
		SET_SAMPLE(Emerald,
				SapphireProps::Fallable | SapphireProps::Round | SapphireProps::Convertable | SapphireProps::Blowable
						| SapphireProps::ObjectPushable | SapphireProps::Pickable);
		SET_SAMPLE(Sapphire,
				SapphireProps::Fallable | SapphireProps::Round | SapphireProps::Convertable | SapphireProps::Blowable
						| SapphireProps::ObjectPushable | SapphireProps::Pickable);
		SET_SAMPLE(Ruby,
				SapphireProps::Fallable | SapphireProps::Round | SapphireProps::Convertable | SapphireProps::Pickable
						| SapphireProps::ObjectPushable);
		SET_SAMPLE(Cushion, SapphireProps::Round | SapphireProps::Blowable | SapphireProps::Pushable | SapphireProps::ObjectPushable);
		SET_SAMPLE(DoorBlue, SapphireProps::Blowable | SapphireProps::Round);
		SET_SAMPLE(DoorRed, SapphireProps::Blowable | SapphireProps::Round);
		SET_SAMPLE(DoorYellow, SapphireProps::Blowable | SapphireProps::Round);
		SET_SAMPLE(DoorGreen, SapphireProps::Blowable | SapphireProps::Round);
		SET_SAMPLE(DoorOneTime, SapphireProps::Blowable | SapphireProps::Round);
		SET_SAMPLE(Earth, SapphireProps::Blowable | SapphireProps::Pickable);
		SET_SAMPLE(Elevator, SapphireProps::Blowable);
		SET_SAMPLE(Glass, SapphireProps::NO_FLAG);
		SET_SAMPLE(KeyBlue, SapphireProps::Round | SapphireProps::Blowable | SapphireProps::Pickable);
		SET_SAMPLE(KeyGreen, SapphireProps::Round | SapphireProps::Blowable | SapphireProps::Pickable);
		SET_SAMPLE(KeyRed, SapphireProps::Round | SapphireProps::Blowable | SapphireProps::Pickable);
		SET_SAMPLE(KeyYellow, SapphireProps::Round | SapphireProps::Blowable | SapphireProps::Pickable);
		SET_SAMPLE(Lorry, SapphireProps::Blowable | SapphireProps::Enemy | SapphireProps::ExplodePropagate);
		SET_SAMPLE(Robot, SapphireProps::Blowable | SapphireProps::Enemy);
		SET_SAMPLE(Dispenser, SapphireProps::Blowable);
		SET_SAMPLE(Rock,
				SapphireProps::Fallable | SapphireProps::Convertable | SapphireProps::Round | SapphireProps::Blowable
						| SapphireProps::Pushable | SapphireProps::SandSinkable | SapphireProps::ObjectPushable);
		/*SET_SAMPLE(RockEmerald,
		 SapphireProps::Fallable | SapphireProps::Convertable | SapphireProps::Round | SapphireProps::Blowable
		 | SapphireProps::Pushable | SapphireProps::SandSinkable | SapphireProps::ObjectPushable);*/
		SET_SAMPLE(StoneWall, SapphireProps::Blowable);
		SET_SAMPLE(Wall, SapphireProps::NO_FLAG);
		SET_SAMPLE(RoundStoneWall, SapphireProps::Round | SapphireProps::Blowable);
		//SET_SAMPLE(RoundStoneWallEmerald, SapphireProps::Round | SapphireProps::Blowable);
		SET_SAMPLE(RoundWall, SapphireProps::Round);
		//SET_SAMPLE(InvisibleWall, SapphireProps::NO_FLAG);
		SET_SAMPLE(InvisibleStoneWall, SapphireProps::Blowable);
		SET_SAMPLE(Sand, SapphireProps::Blowable);
		SET_SAMPLE(SandRock, SapphireProps::Blowable);
		//SET_SAMPLE(SandRockEmerald, SapphireProps::Blowable);
		SET_SAMPLE(Safe, SapphireProps::Blowable | SapphireProps::Pushable | SapphireProps::ObjectPushable);
		SET_SAMPLE(Swamp, SapphireProps::Blowable);
		SET_SAMPLE(Acid, SapphireProps::Blowable);
		SET_SAMPLE(Drop, SapphireProps::Fallable | SapphireProps::Blowable);
		SET_SAMPLE(TimeBomb, SapphireProps::Blowable | SapphireProps::Pickable | SapphireProps::ExplodePropagate);
		SET_SAMPLE(TickBomb, SapphireProps::Blowable | SapphireProps::ExplodePropagate);
		SET_SAMPLE(TNT, SapphireProps::Blowable | SapphireProps::Pickable | SapphireProps::ExplodePropagate);
		SET_SAMPLE(YamYam, SapphireProps::Blowable | SapphireProps::Enemy | SapphireProps::ExplodePropagate);
		SET_SAMPLE(Exit, SapphireProps::Blowable | SapphireProps::Round);
		SET_SAMPLE(Wheel, SapphireProps::Blowable | SapphireProps::Round);
		SET_SAMPLE(PusherLeft, SapphireProps::Blowable);
		SET_SAMPLE(PusherRight, SapphireProps::Blowable);

		sample[(unsigned int) SapphireObject::TickBomb].state = SapphireState::Tick1;
		sample[(unsigned int) SapphireObject::Drop].state = SapphireState::FallDelay;

		sample[(unsigned int) SapphireObject::Rock].setFixedStill();
		//sample[(unsigned int) SapphireObject::RockEmerald].setFixedStill();
		sample[(unsigned int) SapphireObject::Bomb].setFixedStill();
		sample[(unsigned int) SapphireObject::Emerald].setFixedStill();
		sample[(unsigned int) SapphireObject::Sapphire].setFixedStill();
		sample[(unsigned int) SapphireObject::Ruby].setFixedStill();
		sample[(unsigned int) SapphireObject::Citrine].setFixedStill();
		sample[(unsigned int) SapphireObject::Bag].setFixedStill();
		sample[(unsigned int) SapphireObject::Dispenser].setDispenserObject(SapphireObject::Rock);
		sample[(unsigned int) SapphireObject::Player].setPlayerFacing(SapphireDirection::Down);

#if LOGGING_ENABLED
		for (unsigned int i = 0; i < (unsigned int) SapphireObject::_count_of_entries; ++i) {
			if (sample[i].object == SapphireObject::Air && (SapphireObject) i != SapphireObject::Air) {
				THROW()<< "Sapphire value not initialized: " << (SapphireObject(i));
			}
		}
#endif
	}
};
static GameObjectMap SampleMap;

template<typename Stream>
static bool readObjectIdentifier(Stream&& stream, Level::ObjectIdentifier* id) {
	unsigned char val;
	if (!stream.template deserialize<unsigned char>(val)) {
		return false;
	}
	*id = val;
	return true;
}
template<typename Stream>
static bool writeObjectIdentifier(Stream&& stream, const Level::ObjectIdentifier& id) {
	return stream.template serialize<unsigned char>(id);
}

#define OBJECT_IDENTIFIER_AIR (Level::ObjectIdentifier{' '})
#define OBJECT_IDENTIFIER_EMERALD (Level::ObjectIdentifier{'*'})
#define OBJECT_IDENTIFIER_SAPPHIRE (Level::ObjectIdentifier{'$'})
#define OBJECT_IDENTIFIER_RUBY (Level::ObjectIdentifier{'('})
#define OBJECT_IDENTIFIER_CITRINE (Level::ObjectIdentifier{')'})
#define OBJECT_IDENTIFIER_BAG (Level::ObjectIdentifier{'@'})
#define OBJECT_IDENTIFIER_SAFE (Level::ObjectIdentifier{'['})

static Level::GameObject getGameObjectForIdentifier(const Level::ObjectIdentifier& c) {
	switch (c) {
		case 'E':
			return SampleMap[SapphireObject::Exit];
		case '.':
			return SampleMap[SapphireObject::Earth];
		case '1': {
			Level::GameObject result = SampleMap[SapphireObject::Player];
			result.setPlayerId(0);
			return result;
		}
		case '2': {
			Level::GameObject result = SampleMap[SapphireObject::Player];
			result.setPlayerId(1);
			return result;
		}
		case '0':
			return SampleMap[SapphireObject::Rock];
		case '*':
			return SampleMap[SapphireObject::Emerald];
		case ')':
			return SampleMap[SapphireObject::Citrine];
		case '$':
			return SampleMap[SapphireObject::Sapphire];
		case '(':
			return SampleMap[SapphireObject::Ruby];
		case 'a':
			return SampleMap[SapphireObject::Acid];
		case '|':
			return SampleMap[SapphireObject::RoundStoneWall];
		case 'i':
			return SampleMap[SapphireObject::InvisibleStoneWall];
		case 'Q':
			return SampleMap[SapphireObject::Bomb];
		case '!':
			return SampleMap[SapphireObject::TimeBomb];
		case '?': {
			Level::GameObject result = SampleMap[SapphireObject::TickBomb];
			result.state = SapphireState::Tick2;
			return result;
		}
		case '\"':
			return SampleMap[SapphireObject::TNT];
		case '&': {
			Level::GameObject result = SampleMap[SapphireObject::StoneWall];
			result.setStoneWallObject(SapphireObject::Emerald);
			return result;
		}
		case 'A': {
			//previously auxilary animation

			Level::GameObject result = SampleMap[SapphireObject::StoneWall];
			result.setStoneWallObject(SapphireObject::Sapphire);
			return result;
		}
		case 'C': {
			//previously auxilary animation

			Level::GameObject result = SampleMap[SapphireObject::StoneWall];
			result.setStoneWallObject(SapphireObject::Ruby);
			return result;
		}
		case 'J': {
			//previously auxilary animation

			Level::GameObject result = SampleMap[SapphireObject::StoneWall];
			result.setStoneWallObject(SapphireObject::Citrine);
			return result;
		}
		case 'L':
		case 'M':

		case 'N':
		case 'O':
		case 'P':
		case 'T':
		case 'U':

		case 'W':
		case 'X':
		case 'Z': {
			//LOGI()<< "Loading auxilary animation: " << c;
			THROW();
		}
			/* no break */
		case '+':
			return SampleMap[SapphireObject::StoneWall];
		case '_':
			return SampleMap[SapphireObject::Cushion];
		case '~':
		case ' ':
			return SampleMap[SapphireObject::Air];
		case '#':
			return SampleMap[SapphireObject::Wall];
		case 's':
			return SampleMap[SapphireObject::Sand];
		case 'S':
			return SampleMap[SapphireObject::SandRock];
		case '@':
			return SampleMap[SapphireObject::Bag];
		case 'o':
			return SampleMap[SapphireObject::Robot];
		case 'w':
			return SampleMap[SapphireObject::Wheel];
		case '[':
			return SampleMap[SapphireObject::Safe];
		case '/':
			return SampleMap[SapphireObject::Drop];
		case '%':
			return SampleMap[SapphireObject::Swamp];
		case 'c':
			return SampleMap[SapphireObject::Converter];
		case 'd':
			return SampleMap[SapphireObject::Dispenser];
		case 'e': {
			//previously auxilary animation

			Level::GameObject result = SampleMap[SapphireObject::Dispenser];
			result.setDispenserObject(SapphireObject::Emerald);
			return result;
		}
		case 'f': {
			//previously auxilary animation

			Level::GameObject result = SampleMap[SapphireObject::Dispenser];
			result.setDispenserObject(SapphireObject::Sapphire);
			return result;
		}
		case 'D': {
			//previously auxilary animation

			Level::GameObject result = SampleMap[SapphireObject::Dispenser];
			result.setDispenserObject(SapphireObject::Ruby);
			return result;
		}
		case 'F': {
			//previously auxilary animation

			Level::GameObject result = SampleMap[SapphireObject::Dispenser];
			result.setDispenserObject(SapphireObject::Bomb);
			return result;
		}
		case 'H': {
			//previously auxilary animation

			Level::GameObject result = SampleMap[SapphireObject::Dispenser];
			result.setDispenserObject(SapphireObject::Bag);
			return result;
		}
		case 'I': {
			//previously auxilary animation

			Level::GameObject result = SampleMap[SapphireObject::Dispenser];
			result.setDispenserObject(SapphireObject::Drop);
			return result;
		}
		case 'K': {
			//previously auxilary animation

			Level::GameObject result = SampleMap[SapphireObject::Dispenser];
			result.setDispenserObject(SapphireObject::Citrine);
			return result;
		}
		case '=':
			return SampleMap[SapphireObject::DoorOneTime];
		case 'B':
			return SampleMap[SapphireObject::DoorBlue];
		case 'G':
			return SampleMap[SapphireObject::DoorGreen];
		case 'R':
			return SampleMap[SapphireObject::DoorRed];
		case 'Y':
			return SampleMap[SapphireObject::DoorYellow];
		case 'b':
			return SampleMap[SapphireObject::KeyBlue];
		case 'g':
			return SampleMap[SapphireObject::KeyGreen];
		case 'r':
			return SampleMap[SapphireObject::KeyRed];
		case 'y':
			return SampleMap[SapphireObject::KeyYellow];
		case ':':
			return SampleMap[SapphireObject::Glass];
		case 'l':
			return SampleMap[SapphireObject::PusherLeft];
		case 'm':
			return SampleMap[SapphireObject::PusherRight];
		case '{':
			return SampleMap[SapphireObject::Elevator];
		case '^': {
			Level::GameObject result = SampleMap[SapphireObject::YamYam];
			result.direction = SapphireDirection::Up;
			result.setYamYamOldDirection(SapphireDirection::Up);
			return result;
		}
		case 'V': {
			Level::GameObject result = SampleMap[SapphireObject::YamYam];
			result.direction = SapphireDirection::Down;
			result.setYamYamOldDirection(SapphireDirection::Down);
			return result;
		}
		case '<': {
			Level::GameObject result = SampleMap[SapphireObject::YamYam];
			result.direction = SapphireDirection::Left;
			result.setYamYamOldDirection(SapphireDirection::Left);
			return result;
		}
		case '>': {
			Level::GameObject result = SampleMap[SapphireObject::YamYam];
			result.direction = SapphireDirection::Right;
			result.setYamYamOldDirection(SapphireDirection::Right);
			return result;
		}
		case '5': {
			Level::GameObject result = SampleMap[SapphireObject::Lorry];
			result.state = SapphireState::Moving;
			result.direction = SapphireDirection::Left;
			return result;
		}
		case '6': {
			Level::GameObject result = SampleMap[SapphireObject::Lorry];
			result.state = SapphireState::Moving;
			result.direction = SapphireDirection::Up;
			return result;
		}
		case '7': {
			Level::GameObject result = SampleMap[SapphireObject::Lorry];
			result.state = SapphireState::Moving;
			result.direction = SapphireDirection::Right;
			return result;
		}
		case '8': {
			Level::GameObject result = SampleMap[SapphireObject::Lorry];
			result.state = SapphireState::Moving;
			result.direction = SapphireDirection::Down;
			return result;
		}
		case 'h': {
			Level::GameObject result = SampleMap[SapphireObject::Bug];
			result.state = SapphireState::Moving;
			result.direction = SapphireDirection::Left;
			return result;
		}
		case 'u': {
			Level::GameObject result = SampleMap[SapphireObject::Bug];
			result.state = SapphireState::Moving;
			result.direction = SapphireDirection::Up;
			return result;
		}
		case 'k': {
			Level::GameObject result = SampleMap[SapphireObject::Bug];
			result.state = SapphireState::Moving;
			result.direction = SapphireDirection::Right;
			return result;
		}
		case 'j': {
			Level::GameObject result = SampleMap[SapphireObject::Bug];
			result.state = SapphireState::Moving;
			result.direction = SapphireDirection::Down;
			return result;
		}
		default: {
			THROW()<< "Sapphire object not found for: " << (unsigned int) c;
			return SampleMap[SapphireObject::Air];
		}
	}
}
Level::ObjectIdentifier Level::GameObject::mapToIdentifier() const {
	switch (object) {
		case SapphireObject::Exit:
			return 'E';
		case SapphireObject::Earth:
			return '.';
		case SapphireObject::Player: {
			if (getPlayerId() == 0) {
				return '1';
			}
			return '2';
		}
		case SapphireObject::Rock:
			return '0';
		case SapphireObject::Emerald:
			return '*';
		case SapphireObject::Citrine:
			return ')';
		case SapphireObject::Sapphire:
			return '$';
		case SapphireObject::Ruby:
			return '(';
		case SapphireObject::Acid:
			return 'a';
		case SapphireObject::RoundStoneWall:
			return '|';
		case SapphireObject::InvisibleStoneWall:
			return 'i';
		case SapphireObject::Bomb:
			return 'Q';
		case SapphireObject::TimeBomb:
			return '!';
		case SapphireObject::TickBomb:
			return '?';
		case SapphireObject::TNT:
			return '\"';
		case SapphireObject::StoneWall: {
			switch (getStoneWallObject()) {
				case SapphireObject::Emerald: {
					return '&';
				}
				case SapphireObject::Sapphire: {
					return 'A';
				}
				case SapphireObject::Ruby: {
					return 'C';
				}
				case SapphireObject::Citrine: {
					return 'J';
				}
				default: {
					break;
				}
			}
			return '+';
		}
		case SapphireObject::Cushion:
			return '_';
		case SapphireObject::Air:
			return ' ';
		case SapphireObject::Wall:
			return '#';
		case SapphireObject::Sand:
			return 's';
		case SapphireObject::SandRock:
			return 'S';
		case SapphireObject::Bag:
			return '@';
		case SapphireObject::Robot:
			return 'o';
		case SapphireObject::Wheel:
			return 'w';
		case SapphireObject::Safe:
			return '[';
		case SapphireObject::Drop:
			return '/';
		case SapphireObject::Swamp:
			return '%';
		case SapphireObject::Converter:
			return 'c';
		case SapphireObject::Dispenser: {
			switch (getDispenserObject()) {
				case SapphireObject::Rock: {
					return 'd';
				}
				case SapphireObject::Emerald: {
					return 'e';
				}
				case SapphireObject::Sapphire: {
					return 'f';
				}
				case SapphireObject::Ruby: {
					return 'D';
				}
				case SapphireObject::Bomb: {
					return 'F';
				}
				case SapphireObject::Bag: {
					return 'H';
				}
				case SapphireObject::Drop: {
					return 'I';
				}
				case SapphireObject::Citrine: {
					return 'K';
				}
				default: {
					THROW();
					break;
				}
			}
			THROW();
			return 'd';
		}
		case SapphireObject::DoorOneTime:
			return '=';
		case SapphireObject::DoorBlue:
			return 'B';
		case SapphireObject::DoorGreen:
			return 'G';
		case SapphireObject::DoorRed:
			return 'R';
		case SapphireObject::DoorYellow:
			return 'Y';
		case SapphireObject::KeyBlue:
			return 'b';
		case SapphireObject::KeyGreen:
			return 'g';
		case SapphireObject::KeyRed:
			return 'r';
		case SapphireObject::KeyYellow:
			return 'y';
		case SapphireObject::Glass:
			return ':';
		case SapphireObject::PusherLeft:
			return 'l';
		case SapphireObject::PusherRight:
			return 'm';
		case SapphireObject::Elevator:
			return '{';
		case SapphireObject::YamYam: {
			switch (direction) {
				case SapphireDirection::Left: {
					return '<';
				}
				case SapphireDirection::Up: {
					return '^';
				}
				case SapphireDirection::Right: {
					return '>';
				}
				case SapphireDirection::Down: {
					return 'V';
				}
				default: {
					THROW()<< "Invalid direction" << direction;
					break;
				}
			}
			return ' ';
		}
		case SapphireObject::Lorry: {
			switch (direction) {
				case SapphireDirection::Left: {
					return '5';
				}
				case SapphireDirection::Up: {
					return '6';
				}
				case SapphireDirection::Right: {
					return '7';
				}
				case SapphireDirection::Down: {
					return '8';
				}
				default: {
					THROW() << "Invalid direction" << direction;
					break;
				}
			}
			return ' ';
		}
		case SapphireObject::Bug: {
			switch (direction) {
				case SapphireDirection::Left: {
					return 'h';
				}
				case SapphireDirection::Up: {
					return 'u';
				}
				case SapphireDirection::Right: {
					return 'k';
				}
				case SapphireDirection::Down: {
					return 'j';
				}
				default: {
					THROW() << "Invalid direction" << direction;
					break;
				}
			}
			return ' ';
		}
		default: {
			THROW() << "Character not mapped for sapphire object: " << object;
			return ' ';
		}
	}
	return ' ';
}
static unsigned int getLootCountForIdentifier(const Level::ObjectIdentifier& obj) {
	switch (obj) {
		case '*': { //emerald
			return 1;
		}
		case '$': { //sapphire
			return 3;
		}
		case '@': { //bag
			return 1;
		}
		case '(': { //ruby
			return 5;
		}
		case ')': { //citrine
			return 2;
		}
		case '[': { //safe
			return 1;
		}
		case '&': { //stonewallemerald
			return 1;
		}
		case 'A': { //stonewallsapphire
			return 3;
		}
		case 'C': { //stonewallruby
			return 5;
		}
		case 'J': { //stonewallcitrine
			return 2;
		}
		case 'h':
		case 'u':
		case 'k':
		case 'j': { //bugs
			return 11;
		}
		default: {
			return 0;
		}
	}
}

inline static unsigned int getLootCount(const Level::GameObject& obj) {
	switch (obj.object) {
		case SapphireObject::Emerald:
			return 1;
		case SapphireObject::StoneWall: {
			switch (obj.getStoneWallObject()) {
				case SapphireObject::Emerald: {
					return 1;
				}
				case SapphireObject::Sapphire: {
					return 3;
				}
				case SapphireObject::Ruby: {
					return 5;
				}
				case SapphireObject::Citrine: {
					return 2;
				}
				default: {
					break;
				}
			}
			break;
		}
			/*case SapphireObject::RockEmerald:
			 return 1;*/
		case SapphireObject::Safe:
			return 1;
		case SapphireObject::Bag:
			return 1;
		case SapphireObject::Sapphire:
			return 3;
		case SapphireObject::Ruby:
			return 5;
		case SapphireObject::Citrine:
			return 2;
		case SapphireObject::Bug:
			return 11;
		default:
			return 0;
	}
	return 0;
}

inline static SapphireDirection rotateDirection(SapphireDirection dir, SapphireDirection rotatedir) {
	ASSERT(rotatedir == SapphireDirection::Left || rotatedir == SapphireDirection::Right) << "invalid rotate direction " << (rotatedir);
	switch (dir) {
		case SapphireDirection::Left:
			return rotatedir == SapphireDirection::Left ? SapphireDirection::Down : SapphireDirection::Up;
		case SapphireDirection::Up:
			return rotatedir == SapphireDirection::Left ? SapphireDirection::Left : SapphireDirection::Right;
		case SapphireDirection::Right:
			return rotatedir == SapphireDirection::Left ? SapphireDirection::Up : SapphireDirection::Down;
		case SapphireDirection::Down:
			return rotatedir == SapphireDirection::Left ? SapphireDirection::Right : SapphireDirection::Left;
		default: {
			THROW()<< "invalid direction to rotate: " << (dir);
			return SapphireDirection::Undefined;
		}
	}
}

#define IS_Y(y) ((unsigned int)(y) < this->height)
#define IS_X(x) ((unsigned int)(x) < this->width)
#define SET_OBJECT(x,y, obj) MAP(x,y) = SampleMap[SapphireObject::obj];
#define IDX(x, y) ((y) * this->width + (x))
#define MAP(x,y) (this->map[IDX(x,y)])
#define OPT_MAP_X(x, y) (IS_X(x)?&MAP(x,y):nullptr)
#define OPT_MAP_Y(x, y) (IS_Y(y)?&MAP(x,y):nullptr)
#define OPT_BELOW(x, y) OPT_MAP_Y(x,y-1)
#define OPT_ABOVE(x, y) OPT_MAP_Y(x,y+1)
#define OPT_TOLEFT(x, y) OPT_MAP_X(x-1,y)
#define OPT_TORIGHT(x, y) OPT_MAP_X(x+1,y)

Level::Level() {
}
Level::Level(const Level& o)
		: info(o.info), width(o.width), height(o.height), playerCount(o.playerCount), properties(o.properties), currentDispenserValue(
				o.currentDispenserValue), currentElevatorValue(o.currentElevatorValue), closeExitOnEnter(o.closeExitOnEnter), wheelRemainingTurns(
				o.wheelRemainingTurns), openedExists(o.openedExists), targetLoot(o.targetLoot), pickedLoot(o.pickedLoot), minersFinished(
				o.minersFinished), minersTotal(o.minersTotal), minersPlaying(o.minersPlaying), yamyamRemainders(
				o.yamyamRemainderCount > 0 ? new ObjectIdentifier[o.yamyamRemainderCount * 9] : nullptr), yamyamRemainderCount(
				o.yamyamRemainderCount), currentYamyamRemainder(o.currentYamyamRemainder), demos(o.demos), controls(
				new PlayerControl[o.playerCount]), map(new GameObject[o.width * o.height]), demoSteps(new char[o.demoStepsLength]), demoStepsLength(
				o.demoStepsLength), originalRandomSeed(o.originalRandomSeed), random(o.random), keysCollected(o.keysCollected), bombsCollected(
				o.bombsCollected), musicName(o.musicName), lootLost(o.lootLost), statistics(o.statistics), levelVersion(o.levelVersion) {
	if (o.yamyamRemainderCount > 0) {
		memcpy(yamyamRemainders, o.yamyamRemainders, o.yamyamRemainderCount * 9 * sizeof(ObjectIdentifier));
	}
	memcpy(demoSteps, o.demoSteps, o.demoStepsLength * sizeof(char));
	memcpy(map, o.map, o.width * o.height * sizeof(GameObject));

	// do not copy sound turn variables (lastLorry, lastBug)

	// reseted at every turn:
	// soundCount
	// laserId
	// robotTargets

	//query wheel by x y
	if (o.wheel != nullptr) {
		this->wheel = &MAP(o.wheel->x, o.wheel->y);
	}

}
Level& Level::operator=(Level&& o) {
	delete[] map;
	delete[] controls;
	delete[] yamyamRemainders;
	delete[] demoSteps;

	info = util::move(o.info);
	width = util::move(o.width);
	height = util::move(o.height);
	playerCount = util::move(o.playerCount);
	properties = util::move(o.properties);
	currentDispenserValue = util::move(o.currentDispenserValue);
	currentElevatorValue = util::move(o.currentElevatorValue);
	closeExitOnEnter = util::move(o.closeExitOnEnter);
	wheel = util::move(o.wheel);
	wheelRemainingTurns = util::move(o.wheelRemainingTurns);
	openedExists = util::move(o.openedExists);
	targetLoot = util::move(o.targetLoot);
	pickedLoot = util::move(o.pickedLoot);
	minersFinished = util::move(o.minersFinished);
	minersTotal = util::move(o.minersTotal);
	minersPlaying = util::move(o.minersPlaying);
	yamyamRemainders = util::move(o.yamyamRemainders);
	yamyamRemainderCount = util::move(o.yamyamRemainderCount);
	currentYamyamRemainder = util::move(o.currentYamyamRemainder);
	demos = util::move(o.demos);
	controls = util::move(o.controls);
	map = util::move(o.map);
	demoSteps = util::move(o.demoSteps);
	demoStepsLength = util::move(o.demoStepsLength);
	originalRandomSeed = util::move(o.originalRandomSeed);
	random = util::move(o.random);
	keysCollected = util::move(o.keysCollected);
	bombsCollected = util::move(o.bombsCollected);
	musicName = util::move(o.musicName);
	lootLost = util::move(o.lootLost);

	levelVersion = util::move(o.levelVersion);

	lastLorrySoundTurn = -1000;
	lastBugSoundTurn = -1000;

	statistics = util::move(o.statistics);

	return *this;
}
Level& Level::operator=(const Level& o) {
	delete[] map;
	delete[] controls;
	delete[] yamyamRemainders;
	delete[] demoSteps;

	info = o.info;
	width = o.width;
	height = o.height;
	playerCount = o.playerCount;
	properties = o.properties;
	currentDispenserValue = o.currentDispenserValue;
	currentElevatorValue = o.currentElevatorValue;
	closeExitOnEnter = o.closeExitOnEnter;
	wheelRemainingTurns = o.wheelRemainingTurns;
	openedExists = o.openedExists;
	targetLoot = o.targetLoot;
	pickedLoot = o.pickedLoot;
	minersFinished = o.minersFinished;
	minersTotal = o.minersTotal;
	minersPlaying = o.minersPlaying;
	yamyamRemainders = o.yamyamRemainderCount > 0 ? new ObjectIdentifier[o.yamyamRemainderCount * 9] : nullptr;
	yamyamRemainderCount = o.yamyamRemainderCount;
	currentYamyamRemainder = o.currentYamyamRemainder;
	demos = o.demos;
	controls = new PlayerControl[o.playerCount];
	map = new GameObject[o.width * o.height];
	demoSteps = new char[o.demoStepsLength];
	demoStepsLength = o.demoStepsLength;
	originalRandomSeed = o.originalRandomSeed;
	random = o.random;
	keysCollected = o.keysCollected;
	bombsCollected = o.bombsCollected;
	musicName = o.musicName;
	lootLost = o.lootLost;

	levelVersion = o.levelVersion;

	lastLorrySoundTurn = -1000;
	lastBugSoundTurn = -1000;

	statistics = o.statistics;

	if (o.yamyamRemainderCount > 0) {
		memcpy(yamyamRemainders, o.yamyamRemainders, o.yamyamRemainderCount * 9 * sizeof(ObjectIdentifier));
	}
	memcpy(demoSteps, o.demoSteps, o.demoStepsLength * sizeof(char));
	memcpy(map, o.map, o.width * o.height * sizeof(GameObject));

	if (o.wheel != nullptr) {
		this->wheel = &MAP(o.wheel->x, o.wheel->y);
	} else {
		this->wheel = nullptr;
	}

	return *this;
}
Level::~Level() {
	delete[] map;
	delete[] controls;
	delete[] yamyamRemainders;
	delete[] demoSteps;
}
inline bool Level::canFallDown(GameObject& obj) {
	if (!HAS_FLAG(obj.props, SapphireProps::Fallable))
		return false;
	auto* below = OPT_BELOW(obj.x, obj.y);
	if (below == nullptr || below->isAnyExplosion())
		return false;
	if (below->object == SapphireObject::Acid || below->object == SapphireObject::Air)
		return true;
	if (below->object == SapphireObject::Converter && HAS_FLAG(obj.props, SapphireProps::Convertable)) {
		auto* bbelow = OPT_BELOW(below->x, below->y);
		if (bbelow != nullptr && !bbelow->isAnyExplosion() && bbelow->object == SapphireObject::Air)
			return true;
	}
	return false;
}
inline Level::GameObject* Level::getObjectInDirection(SapphireDirection dir, unsigned int x, unsigned int y) {
	switch (dir) {
		case SapphireDirection::Left:
			return OPT_TOLEFT(x, y);
		case SapphireDirection::Up:
			return OPT_ABOVE(x, y);
		case SapphireDirection::Right:
			return OPT_TORIGHT(x, y);
		case SapphireDirection::Down:
			return OPT_BELOW(x, y);
		default: {
			THROW()<< "Invalid direction: " << (dir);
			return nullptr;
		}
	}
}
inline Level::GameObject* Level::getObjectInDirection(SapphireDirection dir, const GameObject& src) {
	return getObjectInDirection(dir, src.x, src.y);
}
void Level::applyControl(unsigned int player, SapphireDirection dir, SapphireControl control) {
	//LOGI() << "Apply control: %u, %s, " <<  player, (dir), (control));
	controls[player].dir = dir;
	controls[player].control = control;
}

#define DIR_X_OPPOSITE(dir) ((dir) == SapphireDirection::Left ? SapphireDirection::Right : SapphireDirection::Left)
#define DIR_Y_OPPOSITE(dir) ((dir) == SapphireDirection::Up ? SapphireDirection::Down : SapphireDirection::Up)
bool Level::tryFallTo(Level::GameObject& o, Level::GameObject& below, Level::GameObject& nextto, Level::GameObject& belownextto,
		SapphireDirection dir, Level::GameObject* belowbelow) {
	if (nextto.canFallSideTo()) {
		if (belownextto.canFallInto()
				|| (belownextto.object == SapphireObject::Converter && HAS_FLAG(o.props, SapphireProps::Convertable)
						&& belowbelow != nullptr && belowbelow->object == SapphireObject::Air && !belowbelow->isAnyExplosion())) {
			switch (o.object) {
				case SapphireObject::Bag:
				case SapphireObject::Bomb:
					addSound(SapphireSound::RollBagBomb, o);
					break;
				case SapphireObject::Emerald:
					addSound(SapphireSound::RollEmerald, o);
					break;
				case SapphireObject::Ruby:
					addSound(SapphireSound::RollRuby, o);
					break;
				case SapphireObject::Sapphire:
					addSound(SapphireSound::RollSapphire, o);
					break;
				case SapphireObject::Citrine:
					addSound(SapphireSound::RollCitrine, o);
					break;
					//case SapphireObject::RockEmerald:
				case SapphireObject::Rock:
					addSound(SapphireSound::RollRock, o);
					break;
				default: {
					break;
				}
			}
			//LOGV("success round falling: %s state: " <<  (o.object), (o.state));
			o.direction = dir;
			o.state = SapphireState::Moving;
			o.setMoving();
			o.setRolling();
			nextto.set(o);
			o.set(SampleMap[SapphireObject::Air]);
			return true;
		}
	}
	return false;
}

void Level::triggerFall(GameObject& o) {
	unsigned int x = o.x;
	unsigned int y = o.y;
	auto* below = OPT_BELOW(x, y);
	//LOGI() << "Trigger fall on: %s (%s) [%s], %u - %u below: %p, %s (%s)", (o.object), (o.state), (o.flags), o.x, o.y,
	//	below, below == nullptr ? "null" : (below->object), below == nullptr ? "null" : (below->state));

	switch (o.object) {
		case SapphireObject::Citrine:
			if (below != nullptr && !below->isAnyExplosion()) {
				switch (below->object) {
					case SapphireObject::Cushion: {
						o.state = SapphireState::Still;
						addSound(SapphireSound::FallCushion, *below);
						below->setFallCushion();
						break;
					}
					case SapphireObject::Bug:
					case SapphireObject::YamYam:
					case SapphireObject::Lorry:
						if (below->canExplodePropagate()) {
							below->setPropagateExplosion(SapphireExplosion::Normal);
						}
						below->setPastObjectFallInto(o.object);
						o.set(SampleMap[SapphireObject::Air]);
						loseLoot(2);
						break;
					case SapphireObject::Robot:
					case SapphireObject::Player:
						below->startExplosion(OBJECT_IDENTIFIER_AIR, SapphireExplosion::Normal);
						below->setPastObjectFallInto(o.object);
						o.set(SampleMap[SapphireObject::Air]);
						addSound(SapphireSound::Explode, *below);
						loseLoot(2);
						break;
					case SapphireObject::Bomb:
						//blow up
						statistics.citrinesBroken++;
						o.set(SampleMap[SapphireObject::Air]);
						o.setCitrineShattered();
						loseLoot(2);
						addSound(SapphireSound::CitrineShatter, o);
						if (below->canExplodePropagate()) {
							below->setPropagateExplosion(SapphireExplosion::Normal);
						}
						break;
					case SapphireObject::Citrine: {
						if (below->state != SapphireState::Still) {
							statistics.citrinesBroken++;
							//break it
							o.set(SampleMap[SapphireObject::Air]);
							o.setCitrineShattered();
							addSound(SapphireSound::CitrineShatter, o);
							loseLoot(2);
						} else {
							//break the other
							statistics.citrinesBroken++;
							o.state = SapphireState::Moving;
							o.setMoving();
							below->set(o);
							o.set(SampleMap[SapphireObject::Air]);
							below->setCitrineBreaking();
							below->direction = SapphireDirection::Down;
							addSound(SapphireSound::CitrineBreak, *below);
							loseLoot(2);
						}
						break;
					}
					default: {
						//break it
						statistics.citrinesBroken++;
						o.set(SampleMap[SapphireObject::Air]);
						o.setCitrineShattered();
						addSound(SapphireSound::CitrineShatter, o);
						loseLoot(2);
						break;
					}
				}
			} else {
				//break it
				statistics.citrinesBroken++;
				o.set(SampleMap[SapphireObject::Air]);
				o.setCitrineShattered();
				addSound(SapphireSound::CitrineShatter, o);
				loseLoot(2);
			}
			break;
			//case SapphireObject::RockEmerald:
		case SapphireObject::Rock: {
			if (below != nullptr && !below->isAnyExplosion()) {
				//LOGI() << "Rock fall below: %s (%s) [%s]", (below->object), (below->state), (below->dynamic));
				switch (below->object) {
					case SapphireObject::Bomb:
						//blow up
						o.state = SapphireState::Still;
						if (below->canExplodePropagate()) {
							below->setPropagateExplosion(SapphireExplosion::Normal);
						}
						break;
					case SapphireObject::Bag: {
						if (below->state != SapphireState::Still) {
							o.state = SapphireState::FallDelay;
						} else {
							bool wasjustpushed = below->isJustPushed();
							SapphireState oldstate = o.state;
							below->set(SampleMap[SapphireObject::Emerald]);
							o.state = SapphireState::Still;
							if (wasjustpushed && oldstate == SapphireState::FallDelay) {
								applyFallableTurn(o);
							}
							statistics.bagsOpened++;
							below->state = SapphireState::BagOpening; //must be after again try
							addSound(SapphireSound::BagOpen, *below);
						}
						break;
					}
					case SapphireObject::Citrine:
						//break it
						if (below->state != SapphireState::Still) {
							o.state = SapphireState::FallDelay;
						} else {
							statistics.citrinesBroken++;
							o.state = SapphireState::Moving;
							o.setMoving();
							below->set(o);
							o.set(SampleMap[SapphireObject::Air]);
							below->setCitrineBreaking();
							below->direction = SapphireDirection::Down;
							addSound(SapphireSound::CitrineBreak, *below);
							loseLoot(2);
						}
						break;
					case SapphireObject::Sapphire:
						if (below->state != SapphireState::Still) {
							o.state = SapphireState::FallDelay;
						} else {
							statistics.sapphiresBroken++;
							o.state = SapphireState::Still;
							below->set(o);
							o.set(SampleMap[SapphireObject::Air]);
							below->setSapphireBreaking();
							below->setMoving();
							below->direction = SapphireDirection::Down;
							addSound(SapphireSound::SapphireBreak, *below);
							loseLoot(3);
						}
						break;
					case SapphireObject::Bug:
					case SapphireObject::YamYam:
					case SapphireObject::Lorry:
						if (below->canExplodePropagate()) {
							below->setPropagateExplosion(SapphireExplosion::Normal);
						}
						below->setPastObjectFallInto(o.object);
						//swallow rock
						o.set(SampleMap[SapphireObject::Air]);
						break;
					case SapphireObject::Robot:
					case SapphireObject::Player:
						below->startExplosion(OBJECT_IDENTIFIER_AIR, SapphireExplosion::Normal);
						below->setPastObjectFallInto(o.object);
						addSound(SapphireSound::Explode, *below);
						o.set(SampleMap[SapphireObject::Air]);
						break;
					case SapphireObject::Cushion:
						addSound(SapphireSound::FallCushion, *below);
						below->setFallCushion();
						o.state = SapphireState::Still;
						break;
					default:
						o.state = SapphireState::Still;
						addSound(SapphireSound::FallRockHard, o);
						break;
				}
			} else {
				o.state = SapphireState::Still;
				addSound(SapphireSound::FallRockHard, o);
			}
			break;
		}
		case SapphireObject::Bomb:
			if (o.canExplodePropagate()) {
				if (below == nullptr || below->isAnyExplosion()) {
					o.setPropagateExplosion(SapphireExplosion::Normal);
				} else if (below->object == SapphireObject::Cushion && !below->isAnyExplosion()) {
					//falling on cushion
				} else if (HAS_FLAG(below->props, SapphireProps::Enemy) || below->object == SapphireObject::Player) {
					o.clearExplodePropagate();
					applyExplosion(o, SapphireExplosion::Normal);
				} else {
					o.setPropagateExplosion(SapphireExplosion::Normal);
				}
			}
			o.state = SapphireState::Still;
			break;
		case SapphireObject::Drop: {
			if (below != nullptr && !below->isAnyExplosion()) {
				switch (below->object) {
					case SapphireObject::Bomb:
						//blow up
						//no swamping up (checked twice)
						o.state = SapphireState::Still;
						if (below->canExplodePropagate()) {
							below->setPropagateExplosion(SapphireExplosion::Normal);
						}
						break;
						/*case SapphireObject::Citrine:
						 //break it
						 o.state = SapphireState::Moving;
						 o.setMoving();
						 below->set(o);
						 o.set(samplemap[SapphireObject::Air]);
						 break;*/
					case SapphireObject::Robot:
					case SapphireObject::Player:
						below->startExplosion(OBJECT_IDENTIFIER_AIR, SapphireExplosion::Normal);
						addSound(SapphireSound::Explode, *below);
						below->setPastObjectFallInto(o.object);
						o.set(SampleMap[SapphireObject::Air]);
						break;
					default:
						if (HAS_FLAG(below->props, SapphireProps::Enemy)) {
							below->setPastObjectFallInto(o.object);
							if (below->canExplodePropagate()) {
								below->setPropagateExplosion(SapphireExplosion::Normal);
							}
							o.set(SampleMap[SapphireObject::Air]);
						} else {
							addSound(SapphireSound::DropStill, o);
							o.set(SampleMap[SapphireObject::Swamp]);
							o.setSwampDropHit();
						}
						break;
				}
			} else {
				addSound(SapphireSound::DropStill, o);
				o.set(SampleMap[SapphireObject::Swamp]);
				o.setSwampDropHit();
			}
			break;
		}
		default: {
			if (below == nullptr || below->isAnyExplosion()) {
				addFallSound(o);
			} else {
				switch (below->object) {
					case SapphireObject::Bomb:
						//blow up
						if (below->canExplodePropagate()) {
							below->setPropagateExplosion(SapphireExplosion::Normal);
						}
						break;
					case SapphireObject::Robot:
					case SapphireObject::Player:
						below->startExplosion(OBJECT_IDENTIFIER_AIR, SapphireExplosion::Normal);
						addSound(SapphireSound::Explode, *below);
						if (HAS_FLAG(o.props, SapphireProps::Blowable)) {
							loseLoot(getLootCount(o));
							below->setPastObjectFallInto(o.object);
							//ruby is not blowable
							o.set(SampleMap[SapphireObject::Air]);
						}
						break;
					case SapphireObject::Citrine:
						//break it
						if (below->state != SapphireState::Still) {
							o.state = SapphireState::FallDelay;
						} else {
							statistics.citrinesBroken++;
							o.state = SapphireState::Moving;
							o.setMoving();
							below->set(o);
							o.set(SampleMap[SapphireObject::Air]);
							below->setCitrineBreaking();
							below->direction = SapphireDirection::Down;
							addSound(SapphireSound::CitrineBreak, *below);
							loseLoot(2);
						}
						break;
					case SapphireObject::Cushion:
						addSound(SapphireSound::FallCushion, *below);
						below->setFallCushion();
						break;
					default:
						if (HAS_FLAG(below->props, SapphireProps::Enemy)) {
							if (HAS_FLAG(o.props, SapphireProps::Blowable)) {
								below->setPastObjectFallInto(o.object);
								loseLoot(getLootCount(o));
								//ruby is not blowable
								o.set(SampleMap[SapphireObject::Air]);
							}
							if (below->canExplodePropagate()) {
								below->setPropagateExplosion(SapphireExplosion::Normal);
							}
						} else {
							addFallSound(o);

						}
						break;
				}
			}
			o.state = SapphireState::Still;
			//THROW() << "Not handled fall trigger: " <<  (o.object));
			break;
		}
	}
}

void Level::applyExplosionState(Level::GameObject& source, Level::GameObject& expo, SapphireObject sourceobject, bool bigexplosion) {
	if (!expo.isAnyExplosion() && !bigexplosion && !HAS_FLAG(expo.props, SapphireProps::Blowable)) {
		//if cant blow
		return;
	}

	//LOGI() << "Explode start: %s at: %u, %u", (expo.object), expo.x, expo.y);
	bool waspropagate = expo.isPropagateExplosion();
	ObjectIdentifier expres;
	if (bigexplosion) {
		expres = OBJECT_IDENTIFIER_AIR;
		if (expo.isAnyExplosion()) {
			loseLoot(getLootCountForIdentifier(expo.getExplosionResult()));
		} else {
			loseLoot(getLootCountForIdentifier(getBlowResult(sourceobject, expo, expo.x - source.x, expo.y - source.y)));
		}
	} else {
		if (expo.isAnyExplosion()) {
			loseLoot(getLootCountForIdentifier(expo.getExplosionResult()));
			expres = getBlowResult(sourceobject, SampleMap[SapphireObject::Air], expo.x - source.x, expo.y - source.y);
		} else {
			expres = getBlowResult(sourceobject, expo, expo.x - source.x, expo.y - source.y);
		}
		if (expo.object == SapphireObject::Safe) {
			statistics.safesOpened++;
		}
	}
	bool hadexplosion = expo.isAnyExplosion();
	expo.startExplosion(expres, bigexplosion ? SapphireExplosion::Big : SapphireExplosion::Normal);
	if (!properties.silentExplosion) {
		addSound(SapphireSound::Explode, expo);
	}

	//LOGI() << "Apply explosion state on %s, %u - %u, flags (%s), hadexp: %d, canprop: %d", (o.object), o.x, o.y, (o.flags),
	//(int )hadexplosion, (int )o.canExplodePropagate());
	if (!hadexplosion && expo.canExplodePropagate()) {
		if (expo.object == SapphireObject::TNT) {
			expo.setPropagateExplosion(SapphireExplosion::Big);
		} else {
			expo.setPropagateExplosion(SapphireExplosion::Normal);
		}
	}
	if (!(waspropagate && expo.turn < this->getTurn())) {
		setObjectTurn(expo);
	}
}
Level::ObjectIdentifier Level::getBlowResult(SapphireObject sourceobject, const GameObject& exploding, int difx, int dify) {
	switch (sourceobject) {
		case SapphireObject::Bug:
			return difx == 0 && dify == 0 ? OBJECT_IDENTIFIER_SAPPHIRE : OBJECT_IDENTIFIER_EMERALD;
		case SapphireObject::YamYam:
			if (yamyamRemainderCount == 0) {
				return OBJECT_IDENTIFIER_EMERALD;
			}
			return yamyamRemainders[currentYamyamRemainder * 9 + 4 + difx - dify * 3];
		default:
			switch (exploding.object) {
				case SapphireObject::StoneWall: {
					if (exploding.isStoneWallWithObject()) {
						switch (exploding.getStoneWallObject()) {
							case SapphireObject::Emerald: {
								return OBJECT_IDENTIFIER_EMERALD;
							}
							case SapphireObject::Sapphire: {
								return OBJECT_IDENTIFIER_SAPPHIRE;
							}
							case SapphireObject::Ruby: {
								return OBJECT_IDENTIFIER_RUBY;
							}
							case SapphireObject::Citrine: {
								return OBJECT_IDENTIFIER_CITRINE;
							}
							default: {
								break;
							}
						}
					}
					break;
				}
				case SapphireObject::Safe:
					return OBJECT_IDENTIFIER_BAG;
				default:
					break;
			}
			break;
	}
	return OBJECT_IDENTIFIER_AIR;
}
inline static SapphireLaser directionToLaser(SapphireDirection dir) {
	switch (dir) {
		case SapphireDirection::Left:
			return SapphireLaser::LaserLeft;
		case SapphireDirection::Up:
			return SapphireLaser::LaserTop;
		case SapphireDirection::Right:
			return SapphireLaser::LaserRight;
		case SapphireDirection::Down:
			return SapphireLaser::LaserBottom;
		default: {
			THROW()<< "invalid direction to laser: " << (dir);
			return SapphireLaser::NO_FLAG;
		}
	}
}
inline static SapphireDirection directionToOpposite(SapphireDirection dir) {
	switch (dir) {
		case SapphireDirection::Left:
			return SapphireDirection::Right;
		case SapphireDirection::Up:
			return SapphireDirection::Down;
		case SapphireDirection::Right:
			return SapphireDirection::Left;
		case SapphireDirection::Down:
			return SapphireDirection::Up;
		default: {
			THROW()<< "invalid direction to opposite: " << (dir);
			return SapphireDirection::Undefined;
		}
	}
}
void Level::startLaser(GameObject& ruby, SapphireDirection dir) {
	statistics.lasersFired++;
	++laserId;
	if (!properties.silentExplosion) {
		addSound(SapphireSound::Laser, ruby);
	}

	GameObject* current = &ruby;
	current->clearLaser();
	current->setLaserId(laserId);
	current->addLaser(directionToLaser(dir));
	while (auto* next = getObjectInDirection(dir, *current)) {
		current = next;
		if (current->getLaserId() == laserId) {
			if (current->hasLaser(directionToLaser(dir))) {
				//LOGI() << "End laser at: " <<  (current->object));
				return;
			}
		} else {
			current->clearLaser();
			current->setLaserId(laserId);
		}
		current->addLaser(directionToLaser(dir)); //in
		if (!current->isAnyExplosion() && !current->isPropagateExplosion()) {
			switch (current->object) {
				case SapphireObject::Emerald:
					switch (dir) {
						case SapphireDirection::Left:
							SET_FLAG(current->visual, SapphireVisual::LaserRightBottom);
							dir = SapphireDirection::Down;
							break;
						case SapphireDirection::Right:
							SET_FLAG(current->visual, SapphireVisual::LaserLeftTop);
							dir = SapphireDirection::Up;
							break;
						case SapphireDirection::Up:
							SET_FLAG(current->visual, SapphireVisual::LaserLeftBottom);
							dir = SapphireDirection::Left;
							break;
						case SapphireDirection::Down:
							SET_FLAG(current->visual, SapphireVisual::LaserRightTop);
							dir = SapphireDirection::Right;
							break;
						default:
							break;
					}
					continue;
				case SapphireObject::Sapphire:
					switch (dir) {
						case SapphireDirection::Left:
							SET_FLAG(current->visual, SapphireVisual::LaserRightTop);
							dir = SapphireDirection::Up;
							break;
						case SapphireDirection::Right:
							SET_FLAG(current->visual, SapphireVisual::LaserLeftBottom);
							dir = SapphireDirection::Down;
							break;
						case SapphireDirection::Up:
							SET_FLAG(current->visual, SapphireVisual::LaserRightBottom);
							dir = SapphireDirection::Right;
							break;
						case SapphireDirection::Down:
							SET_FLAG(current->visual, SapphireVisual::LaserLeftTop);
							dir = SapphireDirection::Left;
							break;
						default:
							break;
					}
					continue;
				case SapphireObject::Glass:
				case SapphireObject::Air:
				case SapphireObject::Ruby:
					goto straight_laser;
				default:
					if (HAS_FLAG(current->props, SapphireProps::Blowable)) {
						applyExplosionState(*current, *current, current->object, false);
					}
					return;
			}
		}
		straight_laser:

		switch (dir) {
			case SapphireDirection::Left:
			case SapphireDirection::Right:
				SET_FLAG(current->visual, SapphireVisual::LaserHorizontal);
				break;
			case SapphireDirection::Up:
			case SapphireDirection::Down:
				SET_FLAG(current->visual, SapphireVisual::LaserVertical);
				break;
			default:
				break;
		}
	}
}
void Level::redirectStartLaser(GameObject& exploding, GameObject& explosionsource) {
	ASSERT(&exploding != &explosionsource) << "trying to start laser from explosion source:  " << explosionsource.x << " - "
			<< explosionsource.x << " " << (explosionsource.object);
	if (exploding.object != SapphireObject::Ruby || exploding.isAnyExplosion())
		return;

	if (exploding.x == explosionsource.x) {
		if (exploding.y < explosionsource.y) {
			startLaser(exploding, SapphireDirection::Down);
		} else {
			startLaser(exploding, SapphireDirection::Up);
		}
	} else if (exploding.y == explosionsource.y) {
		if (exploding.x < explosionsource.x) {
			startLaser(exploding, SapphireDirection::Left);
		} else {
			startLaser(exploding, SapphireDirection::Right);
		}
	}
}
void Level::applyExplosion(GameObject& source, SapphireExplosion type) {
	unsigned int x = source.x;
	unsigned int y = source.y;
//LOGWTF("Apply explosion from: %s, %u, %u", (source.object), x, y);
	SapphireObject sourceobject = source.object;
	const bool bigexplosion = type == SapphireExplosion::Big;

	unsigned int minx = IS_X(x - 1) ? x - 1 : x;
	unsigned int maxx = IS_X(x + 1) ? x + 1 : x;
	unsigned int miny = IS_Y(y - 1) ? y - 1 : y;
	unsigned int maxy = IS_Y(y + 1) ? y + 1 : y;

	ASSERT(source.object != SapphireObject::Ruby) << "Ruby is source of explosion " << x << " " << y;
	applyExplosionState(source, source, sourceobject, bigexplosion);

	if (bigexplosion) {
		if (IS_Y(y + 2)) {
			//top
			for (unsigned int i = minx; i <= maxx; ++i) {
				GameObject& expo = MAP(i, y + 2);
				redirectStartLaser(expo, source);
				applyExplosionState(source, expo, sourceobject, false);
			}
		}
		if (IS_Y(y - 2)) {
			//bottom
			for (unsigned int i = minx; i <= maxx; ++i) {
				GameObject& expo = MAP(i, y - 2);
				redirectStartLaser(expo, source);
				applyExplosionState(source, expo, sourceobject, false);
			}
		}
		if (IS_X(x - 2)) {
			//left
			for (unsigned int i = maxy; i >= miny && IS_Y(i); --i) {
				GameObject& expo = MAP(x - 2, i);
				redirectStartLaser(expo, source);
				applyExplosionState(source, expo, sourceobject, false);
			}
		}
		if (IS_X(x + 2)) {
			//right
			for (unsigned int i = maxy; i >= miny && IS_Y(i); --i) {
				GameObject& expo = MAP(x + 2, i);
				redirectStartLaser(expo, source);
				applyExplosionState(source, expo, sourceobject, false);
			}
		}
	}

	//exlopsion oszoloponkent balrol jobbra, soronkent felulrol lefele
	for (unsigned int i = minx; i <= maxx; ++i) {
		for (unsigned int j = maxy; j >= miny && IS_Y(j); --j) {
			GameObject& expo = MAP(i, j);
			if (&expo == &source)
				continue;

			redirectStartLaser(expo, source);
			applyExplosionState(source, expo, sourceobject, bigexplosion);
		}
	}

	if (sourceobject == SapphireObject::YamYam) {
		if (currentYamyamRemainder + 1 < yamyamRemainderCount)
			++currentYamyamRemainder;
	}
}
void Level::pickObject(GameObject& picker, GameObject& obj, SapphireDirection dir) {
	switch (obj.object) {
		case SapphireObject::Air:
			break;
		case SapphireObject::Earth:
			addSound(SapphireSound::Dig, obj);
			statistics.dirtMined++;
			break;
		case SapphireObject::Emerald:
			addLoot(1);
			addSound(SapphireSound::PickEmerald, obj);
			statistics.emeraldCollected++;
			break;
		case SapphireObject::Sapphire:
			addLoot(3);
			addSound(SapphireSound::PickSapphire, obj);
			statistics.sapphireCollected++;
			break;
		case SapphireObject::Citrine:
			addLoot(2);
			addSound(SapphireSound::PickCitrine, obj);
			statistics.citrineCollected++;
			break;
		case SapphireObject::Ruby:
			addLoot(5);
			addSound(SapphireSound::PickRuby, obj);
			statistics.rubyCollected++;
			break;
		case SapphireObject::KeyRed:
			keysCollected[picker.getPlayerId()] |= SapphireDynamic::KeyRed;
			picker.addRedKey();
			addSound(SapphireSound::PickKey, obj);
			statistics.keysCollected++;
			break;
		case SapphireObject::KeyGreen:
			keysCollected[picker.getPlayerId()] |= SapphireDynamic::KeyGreen;
			picker.addGreenKey();
			addSound(SapphireSound::PickKey, obj);
			statistics.keysCollected++;
			break;
		case SapphireObject::KeyBlue:
			keysCollected[picker.getPlayerId()] |= SapphireDynamic::KeyBlue;
			picker.addBlueKey();
			addSound(SapphireSound::PickKey, obj);
			statistics.keysCollected++;
			break;
		case SapphireObject::KeyYellow:
			keysCollected[picker.getPlayerId()] |= SapphireDynamic::KeyYellow;
			picker.addYellowKey();
			addSound(SapphireSound::PickKey, obj);
			statistics.keysCollected++;
			break;
		case SapphireObject::TimeBomb:
			picker.addBombs(1);
			bombsCollected[picker.getPlayerId()] += 1;
			addSound(SapphireSound::PickBomb, obj);
			statistics.timeBombsCollected++;
			break;
		case SapphireObject::TNT:
			picker.addBombs(10);
			bombsCollected[picker.getPlayerId()] += 10;
			addSound(SapphireSound::PickBomb, obj);
			statistics.timeBombsCollected += 10;
			break;
		default:
			break;
	}

}
Level::GameObject* Level::tryMovePlayerTo(Level::GameObject& player, Level::GameObject& nextto, SapphireDirection dir, bool putbomb) {
//LOGV("Try move player: (%s) to: %s (%s), flags: %s,", (player.dynamic), (nextto.object), (nextto.state),
//	(nextto.flags));
	if (nextto.isAnyExplosion())
		return nullptr;

	if (nextto.object == SapphireObject::Player) {
		actPlayer(nextto);
	}
	if (HAS_FLAG(nextto.props, SapphireProps::Pickable) && (nextto.state == SapphireState::Still)) {
		pickObject(player, nextto, dir);
		switch (nextto.object) {
			case SapphireObject::Air: {
				addSound(SapphireSound::Walk, nextto);
				break;
			}
			case SapphireObject::Earth: {
				player.setPlayerDigging();
				nextto.setPastObjectPicked(SapphireObject::Earth, dir);
				break;
			}
			case SapphireObject::KeyRed:
			case SapphireObject::KeyGreen:
			case SapphireObject::KeyBlue:
			case SapphireObject::KeyYellow:
			case SapphireObject::Citrine:
			case SapphireObject::Ruby:
			case SapphireObject::Sapphire:
			case SapphireObject::Emerald: {
				nextto.setPastObjectPicked(nextto.object, SapphireDirection::Undefined);
				break;
			}
			default: {
				break;
			}
		}

		unsigned int plrid = player.getPlayerId();
		player.direction = dir;
		player.state = SapphireState::Moving;
		player.setMoving();
		nextto.set(player);
		setObjectTurn(nextto);
		bool putbombsuccess = putbomb && nextto.decreaseBombCount();
		player.set(SampleMap[putbombsuccess ? SapphireObject::TickBomb : SapphireObject::Air]);
		if (putbombsuccess) {
			player.setTickBombSpawning();
			statistics.timeBombsSet++;
			--bombsCollected[plrid];
			addSound(SapphireSound::PutBomb, player);
		}
		return &nextto;
	}
	if (nextto.object == SapphireObject::Exit) {
		if (nextto.getExitState() == SapphireDynamic::ExitOpening && nextto.turn < this->getTurn()) {
			nextto.setExitState(SapphireDynamic::ExitOpen);
		}
		if (nextto.getExitState() == SapphireDynamic::ExitOpen) {
			unsigned int plrid = player.getPlayerId();
			nextto.direction = dir;
			setObjectTurn(nextto);
			nextto.setExitState(SapphireDynamic::ExitClosing);
			nextto.setExitWalkPlayer(player.getPlayerId());
			addSound(SapphireSound::ExitClose, nextto);

			bool putbombsuccess = putbomb && player.decreaseBombCount();
			player.set(SampleMap[putbombsuccess ? SapphireObject::TickBomb : SapphireObject::Air]);
			if (putbombsuccess) {
				player.setTickBombSpawning();
				statistics.timeBombsSet++;
				--bombsCollected[plrid];
				addSound(SapphireSound::PutBomb, player);
			}
			++minersFinished;
			--minersPlaying;
			return nullptr;
		}
	}
	if (auto* nextnext = getObjectInDirection(dir, nextto)) {
		if (nextnext->isAnyExplosion())
			return nullptr;
		//LOGI() << "Trying to push: %s, (%s)", (nextto.object), (nextto.state));
		//LOGV("Player move nextnext: %s, " <<  (nextnext->object), (nextnext->flags));
		if (HAS_FLAG(nextto.props, SapphireProps::Pushable) && nextto.state == SapphireState::Still && !nextto.isPropagateExplosion()) {
			auto* abovenext = OPT_ABOVE(nextto.x, nextto.y);
			if (HAS_FLAG(nextto.props, SapphireProps::Fallable) && (dir == SapphireDirection::Up || dir == SapphireDirection::Down)) {
				//trying to push fallable to up or down
				return nullptr;
			}
			if (abovenext != nullptr && abovenext->state == SapphireState::FallDelay)
				return nullptr;
			player.setPlayerTryPush();
			if (nextnext->object == SapphireObject::Player) {
				actPlayer(*nextnext);
			}

			//randoming HAS to be after checking if nextnext is Air
			if (nextnext->object == SapphireObject::Air) {
				player.setPlayerTryPush();
				if (properties.pushRate == 0 || random.next(properties.pushRate) != 0) {
					unsigned int plrid = player.getPlayerId();
					//LOGV("Push: %s, at %u - %u", (nextto.object), x, y);
					player.direction = dir;
					player.state = SapphireState::Pushing;
					player.setMoving();
					nextto.direction = dir;
					nextto.state = SapphireState::Pushing;
					nextto.setMoving();
					nextto.setJustPushed();
					nextto.setRolling();
					setObjectTurn(nextto);

					switch (nextto.object) {
						//case SapphireObject::RockEmerald:
						case SapphireObject::Rock:
							addSound(SapphireSound::PushRock, nextto);
							break;
						case SapphireObject::Bag:
							addSound(SapphireSound::PushBag, nextto);
							break;
						case SapphireObject::Bomb:
							addSound(SapphireSound::PushBomb, nextto);
							break;
						case SapphireObject::Safe:
							addSound(SapphireSound::PushSafe, nextto);
							break;
						case SapphireObject::Cushion:
							addSound(SapphireSound::PushCushion, nextto);
							break;
						default: {
							THROW()<< "Unknown sound for pushed object: " << (nextto.object);
							break;
						}
					}

					nextnext->set(nextto);
					if (!canFallDown(*nextnext)) {
						nextnext->setFixedStill();
					}

					nextto.set(player);
					bool putbombsuccess = putbomb && nextto.decreaseBombCount();
					player.set(SampleMap[putbombsuccess ? SapphireObject::TickBomb : SapphireObject::Air]);
					if (putbombsuccess) {
						player.setTickBombSpawning();
						statistics.timeBombsSet++;
						--bombsCollected[plrid];
						addSound(SapphireSound::PutBomb, player);
					}
					if (auto* below = OPT_BELOW(nextnext->x, nextnext->y)) {
						if (HAS_FLAG(nextnext->props, SapphireProps::Fallable) && !below->isAnyExplosion()) {
							if (canFallDown(*nextnext)) {
								nextnext->state = SapphireState::FallDelay;
							} else {
								nextnext->state = SapphireState::Pushing;
							}
						} else {
							nextnext->state = SapphireState::Pushing;
						}
					} else {
						nextnext->state = SapphireState::Pushing;
					}

					return &nextto;
				}
			}
		} else if (((nextto.object == SapphireObject::DoorRed && player.hasRedKey())
				|| (nextto.object == SapphireObject::DoorBlue && player.hasBlueKey())
				|| (nextto.object == SapphireObject::DoorGreen && player.hasGreenKey())
				|| (nextto.object == SapphireObject::DoorYellow && player.hasYellowKey())
				|| (nextto.object == SapphireObject::DoorOneTime && !nextto.isOneTimeDoorClosed()))) {
			if (nextnext->object == SapphireObject::Player) {
				actPlayer(*nextnext);
			}
			if (nextnext->object == SapphireObject::Air) {
				unsigned int plrid = player.getPlayerId();
				player.direction = dir;
				player.state = SapphireState::Still;
				player.setMoving();
				player.setPlayerUsingDoor();
				nextto.setOneTimeDoorClosed();
				nextto.setUsingDoor();
				setObjectTurn(nextto);
				nextnext->set(player);

				bool putbombsuccess = putbomb && nextnext->decreaseBombCount();
				player.set(SampleMap[putbombsuccess ? SapphireObject::TickBomb : SapphireObject::Air]);
				if (putbombsuccess) {
					player.setTickBombSpawning();
					statistics.timeBombsSet++;
					--bombsCollected[plrid];
					addSound(SapphireSound::PutBomb, player);
				}
				addSound(SapphireSound::UseDoor, nextto);

				return nextnext;
			}
		}
	}
	return nullptr;
}
bool Level::addMoveToPlayer(GameObject& obj) {
	++statistics.moveCount;
	if (getProperties().isMaxStepConstrained()) {
		if (getProperties().maxSteps < statistics.moveCount) {
			return false;
		}
	}
	return true;
}
void Level::actPlayer(GameObject& player) {
	if (player.turn == this->getTurn())
		return;
	setObjectTurn(player);
	if (player.direction != SapphireDirection::Undefined) {
		player.setPlayerFacing(player.direction);
	} else {
		player.setPlayerFacing(SapphireDirection::Down);
	}

	if (turnExplosion(player)) {
		return;
	}

	if (checkPlayerEnemyContact(player)) {
		return;
	}

	if (getProperties().isMaxTimeConstrained() && this->getTurn() > getProperties().maxTime) {
		player.startExplosion(OBJECT_IDENTIFIER_AIR, SapphireExplosion::Normal);
		addSound(SapphireSound::Explode);
		return;
	}

	unsigned int x = player.x;
	unsigned int y = player.y;

	const unsigned int playerid = player.getPlayerId();
	const SapphireDirection direction = controls[playerid].dir;
	const SapphireControl control = controls[playerid].control;
//LOGI() << "Action for player: %u, %u - %u, %s, %s, (%s), bombs: %u", playerid, x, y, (direction), (control),
//	(player.dynamic), player.getBombCount());
	switch (control) {
		case SapphireControl::Move:
		case SapphireControl::PutBomb: {
			if (addMoveToPlayer(player)) {
				if (auto* o2 = getObjectInDirection(direction, player)) {
					if (!o2->isAnyExplosion()) {
						if (o2->object == SapphireObject::Wheel) {
							startWheel(*o2);
						} else if (auto* movedplayer = tryMovePlayerTo(player, *o2, direction, control == SapphireControl::PutBomb)) {
							checkPlayerEnemyContact(*movedplayer);
							if (wheel == nullptr) {
								robotTargets.addToEnd(*new Position { movedplayer->x, movedplayer->y });
							}

							break;
						}
					}
				}
			} else {
				player.startExplosion(OBJECT_IDENTIFIER_AIR, SapphireExplosion::Normal);
				addSound(SapphireSound::Explode);
			}
			goto move_failed;
		}
		case SapphireControl::Take: {
			if (addMoveToPlayer(player)) {
				if (auto* nextto = getObjectInDirection(direction, player)) {
					if (!nextto->isAnyExplosion()) {
						if (nextto->object == SapphireObject::Wheel) {
							startWheel(*nextto);
						} else if (HAS_FLAG(nextto->props, SapphireProps::Pickable)
								&& (nextto->state == SapphireState::Still || nextto->state == SapphireState::BagOpening)) {
							auto oldobj = nextto->object;
							pickObject(player, *nextto, direction);
							nextto->set(SampleMap[SapphireObject::Air]);
							switch (oldobj) {
								case SapphireObject::Earth: {
									player.setPlayerDigging();
									nextto->setPastObjectPicked(SapphireObject::Earth, direction);
									break;
								}
								case SapphireObject::KeyRed:
								case SapphireObject::KeyGreen:
								case SapphireObject::KeyBlue:
								case SapphireObject::KeyYellow:
								case SapphireObject::Citrine:
								case SapphireObject::Ruby:
								case SapphireObject::Sapphire:
								case SapphireObject::Emerald: {
									nextto->setPastObjectPicked(oldobj, direction);
									break;
								}
								default: {
									break;
								}
							}

						}
					}
				}
			} else {
				player.startExplosion(OBJECT_IDENTIFIER_AIR, SapphireExplosion::Normal);
				addSound(SapphireSound::Explode);
			}
			goto move_failed;
		}
		default:
			move_failed: {
				//we can get here if player just went through the exit
				if (player.object == SapphireObject::Player) {
					player.direction = direction;
					player.state = SapphireState::Still;
					if (wheel == nullptr) {
						robotTargets.addToEnd(*new Position { x, y });
					}
				}
				break;
			}
	}
}

void Level::randomYamYamDirection(GameObject& yam) {
	//LOGI() << "Random yamyam: %u - %u", yam.x, yam.y);
	unsigned int randdir = random.next(4);
	switch (randdir) {
		case 0:  //left
			yam.direction = SapphireDirection::Left;
			break;
		case 1:  //right
			yam.direction = SapphireDirection::Right;
			break;
		case 2:  //up
			yam.direction = SapphireDirection::Up;
			break;
		case 3:  //down
			yam.direction = SapphireDirection::Down;
			break;
		default:
			break;
	}
}
void Level::moveYamYam(GameObject& yam) {
	unsigned int x = yam.x;
	unsigned int y = yam.y;

	GameObject* nextto = getObjectInDirection(yam.direction, x, y);

	yam.setYamYamOldDirection(yam.direction);

	if (nextto != nullptr && !nextto->isAnyExplosion()) {
		if (nextto->object == SapphireObject::Air) {
			yam.state = SapphireState::Moving;
			yam.setMoving();
			nextto->set(yam);
			yam.set(SampleMap[SapphireObject::Air]);
		} else if (nextto->object == SapphireObject::Player) {
			//yamyam not moving in this case, blows up in place
			//nextto->set(yam);
			//yam.set(samplemap[SapphireObject::Air]);

			yam.clearExplodePropagate();
			applyExplosion(yam, SapphireExplosion::Normal);
			yam.state = SapphireState::Still;
			yam.direction = SapphireDirection::Undefined;
		} else if (nextto->object == SapphireObject::Sapphire && nextto->state == SapphireState::Still) {
			loseLoot(3);
			yam.state = SapphireState::Taking;
			//direction stays same
			nextto->set(SampleMap[SapphireObject::Air]);
			nextto->setPastObjectPicked(SapphireObject::Sapphire, yam.direction);
		} else {
			goto fallback_not_moving;
		}
	} else {
		//failed to move
		//LOGV("Yamyam nextto: %s (%s) explosion: " <<  (nextto->object), (nextto->state), (nextto->explosionType));
		goto fallback_not_moving;
	}

	return;

	fallback_not_moving: {
		//LOGV("Random yamyam direction: (%s) %u - %u", (yam.state), yam.x, yam.y);
		yam.state = SapphireState::Still;
		randomYamYamDirection(yam);
		if (!properties.silentYamYam) {
			addSound(SapphireSound::YamYam, yam);
		}
		return;
	}
}

void Level::moveEnemyBugLorry(GameObject& o, SapphireDirection defaultturn) {
//if has something next of it, then move, else rotate to right and do a move

	if (o.object == SapphireObject::Lorry && (int) this->getTurn() - lastLorrySoundTurn >= 3) {
		addSound(SapphireSound::LorryMove, o);
		lastLorrySoundTurn = this->getTurn();
	} else if (o.object == SapphireObject::Bug && (int) this->getTurn() - lastBugSoundTurn >= 2) {
		addSound(SapphireSound::BugMove, o);
		lastBugSoundTurn = this->getTurn();
	}

	SapphireDirection underdir = rotateDirection(o.direction, defaultturn);
	SapphireDirection overdir = rotateDirection(o.direction, DIR_X_OPPOSITE(defaultturn));

	auto* frontof = getObjectInDirection(o.direction, o);
	auto* underobj = getObjectInDirection(underdir, o);

	if (underobj == nullptr || underobj->object != SapphireObject::Air || underobj->isAnyExplosion()) {
		//has something under
		//try move forward
		if (frontof == nullptr || frontof->object != SapphireObject::Air || frontof->isAnyExplosion()) {
			//has something in front
			o.direction = rotateDirection(o.direction, DIR_X_OPPOSITE(defaultturn));
			o.state = SapphireState::Still;
		} else {
			//do move

			o.state = SapphireState::Moving;
			o.setMoving();
			frontof->set(o);
			o.set(SampleMap[SapphireObject::Air]);
		}
		o.clearDefaultTurned();
	} else {
		//nothing under
		if (o.state == SapphireState::Still) {
			//was rotating
			if (frontof == nullptr || frontof->object != SapphireObject::Air || frontof->isAnyExplosion()) {
				//has something in front
				if (o.isDefaultTurned()) {
					o.direction = overdir;
					o.clearDefaultTurned();
				} else {
					o.direction = underdir;
					o.setDefaultTurned();
				}
			} else {
				if (o.isDefaultTurned()) {
					o.state = SapphireState::Moving;
					o.setMoving();
					frontof->set(o);
					o.set(SampleMap[SapphireObject::Air]);
					o.clearDefaultTurned();
				} else {
					o.direction = underdir;
					o.setDefaultTurned();
				}
			}
		} else {
			//was moving, rotate
			o.direction = underdir;
			o.state = SapphireState::Still;
			o.setDefaultTurned();
		}
	}
}
void Level::applySwampTurn(GameObject& swamp) {
	if (properties.swampRate == 0) //zero no spreading
		return;

//LOGI() << "Random swamp (%s) %u - %u", (swamp.state), swamp.x, swamp.y);
	if (random.next(this->properties.swampRate) != 0)
		return;

	unsigned int randdir = random.next(4);

	SapphireDirection chosendir;
	switch (randdir) {
		case 0: //up
			chosendir = SapphireDirection::Up;
			break;
		case 1: //down
			chosendir = SapphireDirection::Down;
			break;
		case 2: //left
			chosendir = SapphireDirection::Left;
			break;
		case 3: //right
			chosendir = SapphireDirection::Right;
			break;
		default: {
			THROW()<< "Invalid randomed value: " << randdir;
			return;
		}
	}
	swamp.setSwampHighlight();
	GameObject* nextto = getObjectInDirection(chosendir, swamp);
	addSound(SapphireSound::SwampMove, swamp);

	if (nextto == nullptr || (nextto->object != SapphireObject::Air && nextto->object != SapphireObject::Earth) || nextto->isAnyExplosion())
		return;

	if (nextto->object == SapphireObject::Earth) {
		nextto->setPastObjectPicked(SapphireObject::Earth, chosendir);
	}
	if (chosendir == SapphireDirection::Up) {
		nextto->set(SampleMap[SapphireObject::Swamp]);
		nextto->setSwampSpawnUp();
	} else {
		nextto->set(SampleMap[SapphireObject::Drop]);
		nextto->state = SapphireState::Spawning;
		nextto->direction = chosendir;
		swamp.state = SapphireState::Spawning;
	}
	setObjectTurn(*nextto);
}

void Level::moveRobot(GameObject& robot) {
	if (properties.robotMoveRate == 0)
		return;

	unsigned int x = robot.x;
	unsigned int y = robot.y;

	unsigned int rand = random.next(properties.robotMoveRate);

	if (rand == 0) {
		//find closest
		unsigned int targetx = 0xFFFFFFFF;
		unsigned int targety = 0xFFFFFFFF;

		unsigned int mindif = 0xFFFFFFFF;

		for (auto* node : robotTargets.nodes()) {
			Position& pos = static_cast<Position&>(*node);

			unsigned int dif = abs((int) (x - pos.x)) + abs((int) (y - pos.y));
			if (dif < mindif) {
				mindif = dif;
				targetx = pos.x;
				targety = pos.y;
			}
		}
		//robot csak akkor try move, ha tud?
		if (mindif != 0xFFFFFFFF) {
			int xdif = x - targetx;
			int ydif = y - targety;

			if (xdif != 0) {
				//should move x

				//ydif is more than x dif, try move y
				if (abs(ydif) >= abs(xdif)) {
					if (tryMoveRobot(robot, ydif > 0 ? SapphireDirection::Down : SapphireDirection::Up)) {
						return;
					}
					//failed to move y
					if (tryMoveRobot(robot, xdif > 0 ? SapphireDirection::Left : SapphireDirection::Right)) {
						return;
					}
					//failed to move x y
				} else {
					if (tryMoveRobot(robot, xdif > 0 ? SapphireDirection::Left : SapphireDirection::Right)) {
						return;
					}
					if (ydif != 0 && tryMoveRobot(robot, ydif > 0 ? SapphireDirection::Down : SapphireDirection::Up)) {
						return;
					}
				}
			} else if (ydif != 0) {
				//xdif is 0
				//try move y
				if (tryMoveRobot(robot, ydif > 0 ? SapphireDirection::Down : SapphireDirection::Up)) {
					return;
				}
			}
		}
	}

//not moving
	robot.state = SapphireState::Still;
	robot.direction = SapphireDirection::Undefined;
}

void Level::startWheel(GameObject& wheel) {
	++statistics.wheelsTurned;
	if (this->wheel != nullptr) {
		//stop previous
		this->wheel->clearWheelActive();
	}
	this->wheel = &wheel;
	wheelRemainingTurns = properties.wheelTurnTime;
	wheel.setWheelActive();
	addSound(SapphireSound::Wheel, wheel);
	setObjectTurn(wheel);
	robotTargets.clear();
	robotTargets.addToEnd(*new Position { wheel.x, wheel.y });
}

bool Level::tryMoveRobot(GameObject& robot, SapphireDirection dir) {
	auto* movetarget = getObjectInDirection(dir, robot.x, robot.y);
	if (movetarget == nullptr || movetarget->isAnyExplosion())
		return false;

	if (movetarget->object == SapphireObject::Air) {
		robot.state = SapphireState::Moving;
		robot.setMoving();
		robot.direction = dir;
		movetarget->set(robot);
		robot.set(SampleMap[SapphireObject::Air]);
		addSound(SapphireSound::RobotMove, robot);
		return true;
	} else if (movetarget->object == SapphireObject::Player) {
		robot.state = SapphireState::Moving;
		robot.setMoving();
		robot.direction = dir;
		//explosion result air
		movetarget->startExplosion(OBJECT_IDENTIFIER_AIR, SapphireExplosion::Normal);
		movetarget->setPlayerRobotKill(dir);
		robot.set(SampleMap[SapphireObject::Air]);
		addSound(SapphireSound::Explode, *movetarget);
		addSound(SapphireSound::RobotMove, robot);
		return true;
	}
	return false;
}

void Level::checkLoot() {
	if (openedExists)
		return;

	if (collectedEnoughtLoot()) {
		openedExists = true;
		for (unsigned int j = 0; j < height; ++j) {
			for (unsigned int i = 0; i < width; ++i) {
				GameObject& o = MAP(i, j);
				if (o.object == SapphireObject::Exit) {
					o.setExitState(SapphireDynamic::ExitOpening);
					setObjectTurn(o);
					addSound(SapphireSound::ExitOpen, o);
				}
			}
		}
	}
}

bool Level::turnExplosion(GameObject& o) {
	if (o.isAnyExplosion()) {

		//explode further
		o.increaseExplosionState();
		if (o.getExplosionState() == 1) {
			if (o.object == SapphireObject::Player) {
				--minersPlaying;
			} else if (o.object != SapphireObject::Safe && o.object != SapphireObject::StoneWall && o.object != SapphireObject::Bug) {
				//not one of the above blows, then lose the loot
				//(blowing these will get back the loot
				loseLoot(getLootCount(o));
			}
			o.set(SampleMap[SapphireObject::Air]);
			setObjectTurn(o);
		} else if (o.getExplosionState() == 4) {

			char expres = o.getExplosionResult();
			o.set(getGameObjectForIdentifier(expres));
			setObjectTurn(o);
			o.setExplosionSpawning();

			if (o.object == SapphireObject::Player) {
				++minersTotal;
				++minersPlaying;
			} else if (o.object == SapphireObject::Exit && openedExists) {
				o.setExitState(SapphireDynamic::ExitOpening);
				addSound(SapphireSound::ExitOpen, o);
			}

			o.clearAnyExplosion();
			o.setExplosionResult(0);
			o.clearExplosionState();
		} else {
			o.state = SapphireState::Exploding;
		}
		return true;
	}
	return false;
}

bool Level::checkPlayerEnemyContact(GameObject& player) {
	unsigned int x = player.x;
	unsigned int y = player.y;
	auto* found = OPT_TOLEFT(x, y);
	if (found == nullptr || (found->object != SapphireObject::Lorry && found->object != SapphireObject::Bug) || found->isAnyExplosion()) {
		found = OPT_ABOVE(x, y);
		if (found == nullptr || (found->object != SapphireObject::Lorry && found->object != SapphireObject::Bug)
				|| found->isAnyExplosion()) {
			found = OPT_TORIGHT(x, y);
			if (found == nullptr || (found->object != SapphireObject::Lorry && found->object != SapphireObject::Bug)
					|| found->isAnyExplosion()) {
				found = OPT_BELOW(x, y);
				if (found == nullptr || (found->object != SapphireObject::Lorry && found->object != SapphireObject::Bug)
						|| found->isAnyExplosion()) {
					return false;
				}
			}
		}
	}
	//LOGI() << "Player enemy contact on: %u - %u with: %s player at: %u - %u", found->x, found->y, (found->object), x, y);
	if (found->canExplodePropagate()) {
		//elvileg itt mindig canExplodePropagate kellene hogy legyen
		found->setPropagateExplosion(SapphireExplosion::Normal);
		found->state = SapphireState::Still;
		return true;
	}
	return false;
}

bool Level::checkBugLorrySuicideContact(GameObject& buglorry) {
	unsigned int x = buglorry.x;
	unsigned int y = buglorry.y;
	auto* found = OPT_TOLEFT(x, y);
	if (found == nullptr || (found->object != SapphireObject::Player && found->object != SapphireObject::Swamp)
			|| found->isAnyExplosion()) {
		found = OPT_ABOVE(x, y);
		if (found == nullptr || (found->object != SapphireObject::Player && found->object != SapphireObject::Swamp)
				|| found->isAnyExplosion()) {
			found = OPT_TORIGHT(x, y);
			if (found == nullptr || (found->object != SapphireObject::Player && found->object != SapphireObject::Swamp)
					|| found->isAnyExplosion()) {
				found = OPT_BELOW(x, y);
				if (found == nullptr || (found->object != SapphireObject::Player && found->object != SapphireObject::Swamp)
						|| found->isAnyExplosion()) {
					return false;
				}
			}
		}
	}
	return true;
}
bool Level::checkYamYamSuicideContact(GameObject& yam) {
	unsigned int x = yam.x;
	unsigned int y = yam.y;
	auto* found = OPT_TOLEFT(x, y);
	if (found == nullptr || found->object != SapphireObject::Swamp || found->isAnyExplosion()) {
		found = OPT_ABOVE(x, y);
		if (found == nullptr || found->object != SapphireObject::Swamp || found->isAnyExplosion()) {
			found = OPT_TORIGHT(x, y);
			if (found == nullptr || found->object != SapphireObject::Swamp || found->isAnyExplosion()) {
				found = OPT_BELOW(x, y);
				if (found == nullptr || found->object != SapphireObject::Swamp || found->isAnyExplosion()) {
					return false;
				}
			}
		}
	}
	return true;
}

void Level::applyDispenserTurn(GameObject& dispenser) {
	if (currentDispenserValue == properties.dispenserSpeed) {
		if (auto* below = OPT_BELOW(dispenser.x, dispenser.y)) {
			if (below->object == SapphireObject::Air && !below->isAnyExplosion()) {
				below->set(SampleMap[dispenser.getDispenserObject()]);
				below->state = SapphireState::Dispensing;
				below->setMoving();
				setObjectTurn(*below);
				dispenser.setDispenserSpawn();
			} else {
				goto did_not_spawn;
			}
		} else
			did_not_spawn: {
				dispenser.setDispenserRecharge();
			}
	}
}
inline static SapphireObject getConvertValue(SapphireObject obj) {
	switch (obj) {
		case SapphireObject::Emerald:
			return SapphireObject::Sapphire;
		case SapphireObject::Citrine:
			//stay same
			return SapphireObject::Citrine;
		case SapphireObject::Sapphire:
			return SapphireObject::Rock;
		case SapphireObject::Ruby:
			return SapphireObject::Bag;
		case SapphireObject::Bag:
			return SapphireObject::Ruby;
		case SapphireObject::Rock:
			return SapphireObject::Emerald;
			/*case SapphireObject::RockEmerald:
			 return SapphireObject::Emerald;*/
		default: {
			THROW()<< "Object not convertible: " << (obj);
			return obj;
		}
	}
}
void Level::applyFallableTurn(GameObject& o) {
	unsigned int x = o.x;
	unsigned int y = o.y;
	auto* below = OPT_BELOW(x, y);
	if (below != nullptr && !below->isAnyExplosion()) {
		switch (below->object) {
			case SapphireObject::Air: {
				o.direction = SapphireDirection::Down;
				o.state = SapphireState::Moving;
				o.setMoving();
				o.clearFixedStill();
				below->set(o);
				o.set(SampleMap[SapphireObject::Air]);
				break;
			}
			case SapphireObject::Acid: {
				loseLoot(getLootCount(o));
				addSound(SapphireSound::FallAcid, o);
				below->setPastObjectFallInto(o.object);
				o.set(SampleMap[SapphireObject::Air]);
				break;
			}
			case SapphireObject::Converter: {
				if (HAS_FLAG(o.props, SapphireProps::Convertable) && IS_Y(y - 2)) {
					//convert
					GameObject& belowbelow = MAP(x, y - 2);
					if (belowbelow.object == SapphireObject::Air && !belowbelow.isAnyExplosion()) {
						switch (o.object) {
							case SapphireObject::Bag:
								addSound(SapphireSound::ConvertBag, o);
								break;
							case SapphireObject::Ruby:
								addSound(SapphireSound::ConvertRuby, o);
								break;
							case SapphireObject::Emerald:
								addSound(SapphireSound::ConvertEmerald, o);
								break;
							case SapphireObject::Sapphire:
								addSound(SapphireSound::ConvertSapphire, o);
								break;
								//case SapphireObject::RockEmerald:
							case SapphireObject::Rock:
								addSound(SapphireSound::ConvertRock, o);
								break;
							case SapphireObject::Citrine: {
								addSound(SapphireSound::ConvertCitrine, o);
								break;
							}
							default: {
								THROW()<< "No sound for convert: " << (o.object);
								break;
							}
						}
						below->setPastObjectFallInto(o.object);
						o.set(SampleMap[getConvertValue(o.object)]);
						o.state = SapphireState::Converting;
						o.direction = SapphireDirection::Down;
						o.clearFixedStill();
						belowbelow.set(o);
						belowbelow.setMoving();
						o.set(SampleMap[SapphireObject::Air]);
						statistics.itemsConverted++;
					}
				}
				break;
			}
			case SapphireObject::Cushion: {
				if (o.object == SapphireObject::Drop) {
					break;
				}
				if (o.state != SapphireState::Still) {
					//play sound only if still
					o.state = SapphireState::Still;
					addSound(SapphireSound::FallCushion, *below);
					below->setFallCushion();
				}
			}
			//no break
			default: {
				if (HAS_FLAG(below->props, SapphireProps::Round) && (o.state == SapphireState::Still)
						&& (below->state == SapphireState::Still
								|| (below->state == SapphireState::Pushing || below->object == SapphireObject::Cushion))) {
					auto* above = OPT_ABOVE(x, y);
					if (above == nullptr || above->state != SapphireState::FallDelay) {
						//if has above fall delayed, do not round fall
						if (!IS_X(x - 1)
								|| !tryFallTo(o, *below, MAP(x - 1, y), MAP(x - 1, y - 1), SapphireDirection::Left,
										OPT_MAP_Y(x - 1, y - 2))) {
							//no object on left, or failed to fall left
							//try falling right
							if (auto* toright = OPT_TORIGHT(x, y)) {
								//cant fall to right, even when there is a finishing explosion
								if (!toright->isAnyExplosion()) {
									tryFallTo(o, *below, *toright, MAP(x + 1, y - 1), SapphireDirection::Right, OPT_MAP_Y(x + 1, y - 2));
								}
							}
						}
					}
				} else if (below->object == SapphireObject::Sand && HAS_FLAG(o.props, SapphireProps::SandSinkable)) {
					if (o.state != SapphireState::Still) {
						o.state = SapphireState::Still;
						ASSERT(o.object == SapphireObject::Rock/* || o.object == SapphireObject::RockEmerald*/)
						<< "invalid sandsinkable object " << (o.object);
						addSound(SapphireSound::FallRockSoft, o);
					} else if (below->state != SapphireState::Still) {
						below->state = SapphireState::Still;
					} else {
						//TODO ezt inkabb flagban tarolni h mi van a sandban
						if (o.object == SapphireObject::Rock) {
							below->object = SapphireObject::SandRock;
							below->setPastObjectFallInto(o.object);
						} else {
							/*below->object = SapphireObject::SandRockEmerald;
							 below->setPastObjectFallInto(o.object);*/
						}
						below->clearFixedStill();
						o.set(SampleMap[SapphireObject::Air]);
					}
				}
				break;
			}
		}
	}
	if (o.object != SapphireObject::Air) {
		if ((o.state == SapphireState::Moving || o.state == SapphireState::FallDelay)) {
			triggerFall(o);
		}
		o.setFixedStill();
	}
}

void Level::applyPusherObjectTurn(GameObject& obj, SapphireDirection dir) {
	if (obj.object == SapphireObject::Elevator && (properties.elevatorSpeed == 0 || properties.elevatorSpeed != currentElevatorValue)) {
		obj.state = SapphireState::Still;
		return;
	}
	auto* indir = getObjectInDirection(dir, obj);
	//elevator only fallable
	//pusher safe and cushion too&
	if (indir == nullptr || indir->isAnyExplosion() || !HAS_FLAG(indir->props, SapphireProps::ObjectPushable)
			|| (obj.object == SapphireObject::Elevator && (!HAS_FLAG(indir->props, SapphireProps::Fallable)))
			|| indir->state != SapphireState::Still || indir->isPropagateExplosion() || (canFallDown(*indir) & !indir->isFixedStill())
			|| (HAS_FLAG(indir->props, SapphireProps::SandSinkable) && !canSandSinkablePush(*indir))) {
		//try backward
		auto* backward = getObjectInDirection(directionToOpposite(dir), obj);
		if (backward != nullptr && backward->object == SapphireObject::Air && !backward->isAnyExplosion()
				&& (indir == nullptr || indir->state != SapphireState::Pushing || !HAS_FLAG(indir->props, SapphireProps::ObjectPushable))) {
			if (obj.object == SapphireObject::Elevator) {
				addSound(SapphireSound::Elevator, obj);
			}
			//can backward
			obj.state = SapphireState::Moving;
			obj.setMoving();
			obj.direction = directionToOpposite(dir);
			backward->set(obj);
			obj.set(SampleMap[SapphireObject::Air]);
		} else {
			//cant backward
			obj.state = SapphireState::Still;
		}
	} else {
		//has pushable nextto
		auto* nextnext = getObjectInDirection(dir, *indir);
		if (nextnext != nullptr && nextnext->object == SapphireObject::Air && !nextnext->isAnyExplosion()) {
			if (obj.object == SapphireObject::Elevator) {
				addSound(SapphireSound::Elevator, obj);
			} else {
				//it is hand pusher
				switch (indir->object) {
					//case SapphireObject::RockEmerald:
					case SapphireObject::Rock:
						addSound(SapphireSound::PushRock, *indir);
						break;
					case SapphireObject::Bag:
						addSound(SapphireSound::PushBag, *indir);
						break;
					case SapphireObject::Bomb:
						addSound(SapphireSound::PushBomb, *indir);
						break;
					case SapphireObject::Emerald:
						addSound(SapphireSound::RollEmerald, *indir);
						break;
					case SapphireObject::Sapphire:
						addSound(SapphireSound::RollSapphire, *indir);
						break;
					case SapphireObject::Ruby:
						addSound(SapphireSound::RollRuby, *indir);
						break;
					case SapphireObject::Cushion:
						addSound(SapphireSound::PushCushion, *indir);
						break;
					case SapphireObject::Safe:
						addSound(SapphireSound::PushSafe, *indir);
						break;
					default: {
						THROW()<< "Unknown sound for pushed object: " << (indir->object);
						break;
					}
				}
			}
			//can push
			indir->state = SapphireState::Pushing;
			indir->direction = dir;
			indir->setJustPushed();
			indir->setMoving();
			if (dir == SapphireDirection::Left || dir == SapphireDirection::Right) {
				//do not roll on elevator
				indir->setRolling();
			}
			nextnext->set(*indir);
			obj.state = SapphireState::Moving;
			obj.setMoving();
			obj.direction = dir;
			indir->set(obj);
			obj.set(SampleMap[SapphireObject::Air]);
			setObjectTurn(*nextnext);

			if (!canFallDown(*nextnext)) {
				nextnext->setFixedStill();
			} else {
				nextnext->clearFixedStill();
			}
		} else {
			//cant push, keep pos
			obj.state = SapphireState::Still;
		}
	}
}

bool Level::canSandSinkablePush(GameObject& obj) {
	auto* below = OPT_BELOW(obj.x, obj.y);
	if (below == nullptr || below->isAnyExplosion() || below->object != SapphireObject::Sand || below->state == SapphireState::Spawning
			|| below->state == SapphireState::Dispensing) {
		return true;
	}
	return false;
}

void Level::addSound(SapphireSound sound, const GameObject& obj) {
	int idx = soundsIndex[(unsigned int) sound];
	if (idx < 0) {
		idx = soundCount++;
		sounds[idx] = {sound, obj.x, obj.y};
		soundsIndex[(unsigned int) sound] = idx;
	} else {
		//TODO interpolate between locations?
		sounds[idx].addLocation(obj.x, obj.y);
	}
}
void Level::addSound(SapphireSound sound) {
	int idx = soundsIndex[(unsigned int) sound];
	if (idx < 0) {
		idx = soundCount++;
		sounds[idx] = {sound, 0, 0};
		soundsIndex[(unsigned int) sound] = idx;
	}
}

void Level::applyTurn() {
	if (currentDispenserValue == properties.dispenserSpeed) {
		currentDispenserValue = 1;
	} else {
		++currentDispenserValue;
	}
	if (currentElevatorValue == properties.elevatorSpeed) {
		currentElevatorValue = 1;
	} else {
		++currentElevatorValue;
	}

	unsigned int demoturn = this->getTurn();

	if (demoStepsLength < (demoturn + 1) * this->playerCount) {
		int nlen = demoStepsLength;
		do {
			nlen *= 2;
		} while (nlen < (demoturn + 1) * this->playerCount);
		char* ndemosteps = new char[nlen];
		for (int i = 0; i < demoStepsLength; ++i) {
			ndemosteps[i] = demoSteps[i];
		}
		delete[] demoSteps;
		demoSteps = ndemosteps;
		demoStepsLength = nlen;
	}

	++this->turnRef();
	laserId = 0;
	soundCount = 0;

	for (unsigned int i = 0; i < (unsigned int) SapphireSound::_count_of_entries; ++i) {
		soundsIndex[i] = -1;
	}

	robotTargets.clear();

	for (unsigned int j = 0; j < height; ++j) {
		for (unsigned int i = 0; i < width; ++i) {
			GameObject& o = MAP(i, j);
			bool dispenspawn = o.object == SapphireObject::Dispenser && o.isDispenserSpawn();
			bool exitwalk = o.object == SapphireObject::Exit && o.isExitWalkPlayer();
			o.clearVisual();

			if (dispenspawn) {
				o.setDispenserRecharge();
			} else if (exitwalk) {
				o.setExitSinkPlayer(o.getPlayerId());
			}

			if (o.state == SapphireState::BagOpening) {
				o.state = SapphireState::Still;
			} else if (o.state == SapphireState::Spawning || o.state == SapphireState::Dispensing) {
				if (HAS_FLAG(o.props, SapphireProps::Fallable)) {
					o.state = SapphireState::Moving;
					o.direction = SapphireDirection::Down;
				} else {
					o.state = SapphireState::Still;
				}
			} else if (o.state == SapphireState::Converting) {
				o.state = SapphireState::Moving;
				o.direction = SapphireDirection::Down;
			} else if (o.state == SapphireState::Pushing) {
				o.state = SapphireState::Still;
			} else {
				o.clearJustPushed();
			}
			o.clearLaser();
		}
	}

	if (wheel != nullptr) {
		if (wheel->object != SapphireObject::Wheel || wheel->isAnyExplosion()) {
			//blown up?
			wheel = nullptr;
		} else {
			unsigned int turns = wheelRemainingTurns;
			--wheelRemainingTurns;
			if (turns <= 1) {
				wheel = nullptr;
			} else {
				wheel->setWheelActive();
				robotTargets.addToEnd(*new Position { wheel->x, wheel->y });
				setObjectTurn(*wheel);
				addSound(SapphireSound::Wheel, *wheel);
			}
		}
	}

	for (unsigned int j = 0; j < height; ++j) {
		for (unsigned int i = 0; i < width; ++i) {
			GameObject& o = MAP(i, j);
			if (o.object == SapphireObject::Player) {
				//handles turn equality
				actPlayer(o);
			}
		}
	}

	for (unsigned int i = 0; i < this->playerCount; ++i) {
		demoSteps[demoturn * this->playerCount + i] = DemoPlayer::encodeDemoMove(controls[i].dir, controls[i].control);
		controls[i] = PlayerControl { };
	}

	if (getProperties().isMaxTimeConstrained() && this->getTurn() <= getProperties().maxTime) {
		unsigned int diff = (getProperties().maxTime - this->getTurn());
		if (diff <= 20 && diff % 4 == 0) {
			addSound(SapphireSound::Clock);
		}
	}

	for (unsigned int j = 0; j < height; ++j) {
		for (unsigned int i = 0; i < width; ++i) {
			GameObject& o = MAP(i, j);
			if (o.turn == this->getTurn()) {
				continue;
			}
			ASSERT(o.turn < this->getTurn()) << "invalid turn: " << o.turn << " this->getTurn(): " << this->getTurn() << " at: " << i
					<< " - " << j;
			setObjectTurn(o);

			if (o.isPropagateExplosion()) {
				//LOGD("Execute propagate explosion: %s, %u - %u", (o.object), i, j);
				applyExplosion(o, o.getPropagateExplosion());
				o.clearPropagateExplosion();
				continue;
			}

			if (turnExplosion(o)) {
				continue;
			}

			if (o.isAnyExplosion())
				continue;

			if (o.object == SapphireObject::Exit) {
				//this has to be before object turns
				//because explosions can modulate turn, and the exit gets skipped.
				//its important to keep up with the visuals, and explosion doesnt modify exit state
				switch (o.getExitState()) {
					case SapphireDynamic::ExitOpening: {
						o.setExitState(SapphireDynamic::ExitOpen);
						break;
					}
					case SapphireDynamic::ExitClosing: {
						if (closeExitOnEnter || isSuccessfullyOver()) {
							o.setExitState(SapphireDynamic::ExitOccupied);
						} else {
							o.setExitState(SapphireDynamic::ExitOpen);
						}
						break;
					}
					case SapphireDynamic::ExitOccupied: {
						o.setExitState(SapphireDynamic::ExitClosed);
						break;
					}
					default: {
						break;
					}
				}
			}

			if (o.object == SapphireObject::TickBomb && o.state != SapphireState::Still) {
				if (o.state == SapphireState::TickMax) {
					o.clearExplodePropagate();
					applyExplosion(o, SapphireExplosion::Normal);
					o.state = SapphireState::Still;
					continue;
				} else {
					o.state = SapphireState((unsigned int) o.state + 1);
					addSound(SapphireSound::BombTick, o);
				}
			}
			if (o.object == SapphireObject::Dispenser) {
				applyDispenserTurn(o);
				continue;
			}
			if (HAS_FLAG(o.props, SapphireProps::Fallable)) {
				applyFallableTurn(o);
				continue;
			}
			if (HAS_FLAG(o.props, SapphireProps::Enemy)) {
				switch (o.object) {
					case SapphireObject::YamYam:
						if (checkYamYamSuicideContact(o)) {
							o.clearExplodePropagate();
							applyExplosion(o, SapphireExplosion::Normal);
							o.state = SapphireState::Still;
						} else {
							moveYamYam(o);
						}
						break;
					case SapphireObject::Bug:
						if (checkBugLorrySuicideContact(o)) {
							o.clearExplodePropagate();
							applyExplosion(o, SapphireExplosion::Normal);
						} else {
							moveEnemyBugLorry(o, SapphireDirection::Right);
						}
						break;
					case SapphireObject::Lorry:
						if (checkBugLorrySuicideContact(o)) {
							o.clearExplodePropagate();
							applyExplosion(o, SapphireExplosion::Normal);
						} else {
							moveEnemyBugLorry(o, SapphireDirection::Left);
						}
						break;
					case SapphireObject::Robot:
						moveRobot(o);
						break;
					default: {
						THROW()<< "unhandled enemy: " << (o.object);
						break;
					}
				}
				continue;
			}
			if (o.object == SapphireObject::Swamp) {
				applySwampTurn(o);
				continue;
			}
			if (o.object == SapphireObject::SandRock) {
				//uj SY-ben SandRock Acid-ba beleesik, converterbe nem
				//regiben egyikbe sem
				if (auto* below = OPT_BELOW(i, j)) {
					if (!below->isAnyExplosion()) {
						if (below->object == SapphireObject::Air) {
							o.state = SapphireState::Spawning;
							o.object = SapphireObject::Sand;
							below->set(SampleMap[SapphireObject::Rock]);
							below->state = SapphireState::Moving;
							below->setMoving();
							below->direction = SapphireDirection::Down;
						} else if (below->object == SapphireObject::Sand) {
							o.object = SapphireObject::Sand;
							o.state = SapphireState::Spawning;
							below->object = SapphireObject::SandRock;
							below->setPastObjectFallInto(SapphireObject::Rock);
						}
					}
				}
				continue;
			}
			/*if (o.object == SapphireObject::SandRockEmerald) {
			 if (auto* below = OPT_BELOW(i, j)) {
			 if (below->object == SapphireObject::Air) {
			 o.object = SapphireObject::Sand;
			 o.state = SapphireState::Spawning;
			 below->set(samplemap[SapphireObject::RockEmerald]);
			 below->state = SapphireState::Moving;
			 below->setMoving();
			 below->direction = SapphireDirection::Down;
			 } else if (below->object == SapphireObject::Sand) {
			 o.object = SapphireObject::Sand;
			 o.state = SapphireState::Spawning;
			 below->object = SapphireObject::SandRockEmerald;
			 }
			 }
			 continue;
			 }*/
			if (o.object == SapphireObject::Elevator) {
				applyPusherObjectTurn(o, SapphireDirection::Up);
				continue;
			}
			if (o.object == SapphireObject::PusherLeft) {
				applyPusherObjectTurn(o, SapphireDirection::Left);
				continue;
			}
			if (o.object == SapphireObject::PusherRight) {
				applyPusherObjectTurn(o, SapphireDirection::Right);
				continue;
			}

		}
	}

	checkLoot();
}
void Level::saveLevel(OutputStream& os, bool includeuserdemos) const {
	auto stream = EndianOutputStream<Endianness::Big>::wrap(os);

	unsigned int version = getLevelVersion();

	if (version < 3 && (getProperties().isMaxStepConstrained() || getProperties().isMaxTimeConstrained())) {
		version = 3;
	}
	if (getInfo().nonModifyAbleFlag) {
		//this flag was introduced in version 5, @ steam
		version = 5;
	}

	//VERSION
	stream.serialize<uint32>(version);

	stream.serialize<char>(SAPPHIRE_CMD_DEMOCOUNT);
	stream.serialize<uint32>(includeuserdemos ? getDemoCount() : getNonUserDemoCount());

	stream.serialize<char>('n');
	stream.serialize<FixedString>(getInfo().title);

	stream.serialize<char>('D');
	stream.serialize<uint32>((uint32) getInfo().difficulty);

	stream.serialize<char>('C');
	stream.serialize<uint32>((uint32) getInfo().category);

	stream.serialize<char>(SAPPHIRE_CMD_PLAYERCOUNT);
	stream.serialize<uint32>(getMapPlayerCount());

	stream.serialize<char>(SAPPHIRE_CMD_UUID);
	stream.serialize<SapphireUUID>(getInfo().uuid);

	stream.serialize<char>('a');
	stream.serialize<FixedString>(getInfo().author.getUserName());

	stream.serialize<char>(SAPPHIRE_CMD_LEADERBOARDS);
	stream.serialize<SapphireLeaderboards>(getProperties().leaderboards);

	if (getInfo().nonModifyAbleFlag) {
		stream.serialize<char>(SAPPHIRE_CMD_NON_MODIFYABLE_FLAG);
	}

	if (getProperties().silentYamYam) {
		stream.serialize<char>('Y');
	}
	if (getProperties().silentExplosion) {
		stream.serialize<char>('L');
	}

	stream.serialize<char>('i');
	stream.serialize<FixedString>(getInfo().description);

	if (getProperties().isMaxStepConstrained()) {
		stream.serialize<char>('t');
		stream.serialize<int32>(getProperties().maxSteps);
	}
	if (getProperties().isMaxTimeConstrained()) {
		stream.serialize<char>('T');
		stream.serialize<int32>(getProperties().maxTime);
	}

	if (musicName != nullptr) {
		stream.serialize<char>(',');
		stream.serialize<FixedString>(musicName);
	}

	stream.serialize<char>('E');
	stream.serialize<int32>(getProperties().maxLootLose);

	stream.serialize<char>('p');
	stream.serialize<uint32>(getProperties().pushRate);

	stream.serialize<char>('d');
	stream.serialize<uint32>(getProperties().dispenserSpeed);

	stream.serialize<char>('s');
	stream.serialize<uint32>(getProperties().swampRate);

	stream.serialize<char>('o');
	stream.serialize<uint32>(getProperties().robotMoveRate);

	stream.serialize<char>('w');
	stream.serialize<uint32>(getProperties().wheelTurnTime);

	stream.serialize<char>('e');
	stream.serialize<int32>(getProperties().loot);

	stream.serialize<char>('v');
	stream.serialize<uint32>(getProperties().elevatorSpeed);

	stream.serialize<char>('m');
	stream.serialize<uint32>(getWidth());
	stream.serialize<uint32>(getHeight());
	for (unsigned int j = 0; j < getHeight(); ++j) {
		for (unsigned int i = 0; i < getWidth(); ++i) {
			const Level::GameObject& obj = get(i, getHeight() - 1 - j);
			writeObjectIdentifier(stream, obj.mapToIdentifier());
		}
	}

	if (getYamYamRemainderCount() > 0) {
		stream.serialize<char>('y');
		stream.serialize<uint32>(getYamYamRemainderCount());
		for (int i = 0; i < yamyamRemainderCount * 9; ++i) {
			writeObjectIdentifier(stream, yamyamRemainders[i]);
		}
	}

	for (unsigned int i = 0; i < getDemoCount(); ++i) {
		const Demo* d = getDemo(i);
		if (!includeuserdemos && d->userDemo) {
			continue;
		}
		stream.serialize<char>('R');
		stream.serialize<uint32>(d->randomseed);
		stream.serialize<FixedString>(d->info.title);
		stream.serialize<FixedString>(d->moves);
	}

	stream.serialize<char>(SAPPHIRE_CMD_END_OF_FILE);
}
void Level::saveLevel(FileDescriptor& fd, bool includeuserdemos) const {
	auto out = fd.openOutputStream();
	saveLevel(out, includeuserdemos);
}
bool Level::loadLevel(InputStream& is) {
	for (unsigned int i = 0; i < (unsigned int) SapphireSound::_count_of_entries; ++i) {
		soundsIndex[i] = -1;
	}
	soundCount = 0;

	delete[] map;
	delete[] controls;
	delete[] yamyamRemainders;
	delete[] demoSteps;

	info = {};
	width = 0;
	height = 0;

	playerCount = 1;

	laserId = 0;

	properties = LevelProperties();

	currentDispenserValue = 0;
	currentElevatorValue = 0;

	wheel = nullptr;

	openedExists = false;
	pickedLoot = 0;

	minersFinished = 0;
	minersTotal = 0;

	minersPlaying = 0;

	yamyamRemainders = nullptr;
	yamyamRemainderCount = 0;
	currentYamyamRemainder = 0;

	demos.clear();

	controls = nullptr;

	map = nullptr;

	robotTargets.clear();

	keysCollected.reset();
	bombsCollected.reset();

	demoStepsLength = 128;
	demoSteps = new char[demoStepsLength];

	lootLost = 0;

	statistics = LevelStatistics { };

	unsigned int startexitcount = 0;

	unsigned int countedloot = 0;

	auto stream = EndianInputStream<Endianness::Big>::wrap(is);

	uint32 version;
	if (!stream.deserialize<uint32>(version)) {
		return false;
	}

	this->levelVersion = version;

	if (version > SAPPHIRE_RELEASE_VERSION_NUMBER || version > SAPPHIRE_LEVEL_VERSION_NUMBER) {
		return false;
	}
	while (true) {
		char cmd = 0;
		if (!stream.deserialize<char>(cmd)) {
			break;
		}
		switch (cmd) {
			case SAPPHIRE_CMD_UUID: {
				if (!stream.deserialize<SapphireUUID>(info.uuid)) {
					return false;
				}
				break;
			}
			case SAPPHIRE_CMD_NON_MODIFYABLE_FLAG: {
				info.nonModifyAbleFlag = true;
				break;
			}
			case SAPPHIRE_CMD_DEMOCOUNT: { //demo count
				uint32 democount;
				if (!stream.deserialize<uint32>(democount)) {
					return false;
				}
				break;
			}
			case SAPPHIRE_CMD_PLAYERCOUNT: { // player count
				if (!stream.deserialize<uint32>(this->playerCount)) {
					return false;
				}
				break;
			}
			case 'Y': { //silent yamyam
				properties.silentYamYam = true;
				break;
			}
			case 'L': { //silent laser and explosion
				properties.silentExplosion = true;
				break;
			}
			case 'i': { //description
				if (!stream.deserialize<SafeFixedString<SAPPHIRE_LEVEL_DESCRIPTION_MAX_LEN>>(info.description)) {
					return false;
				}
				break;
			}
			case 'C': { //category
				uint32 cat = (uint32) SapphireLevelCategory::None;
				if (!stream.deserialize<uint32>(cat)) {
					return false;
				}
				if (cat < (uint32) SapphireLevelCategory::_count_of_entries) {
					info.category = (SapphireLevelCategory) cat;
				}
				break;
			}
			case 'D': { //difficulty
				uint32 diff = (uint32) SapphireDifficulty::Unrated;
				if (!stream.deserialize<uint32>(diff) || (unsigned int) diff >= (unsigned int) SapphireDifficulty::_count_of_entries) {
					return false;
				}
				info.difficulty = (SapphireDifficulty) diff;
				break;
			}
			case 'n': { // title
				if (!stream.deserialize<SafeFixedString<SAPPHIRE_LEVEL_TITLE_MAX_LEN>>(info.title)) {
					return false;
				}
				break;
			}
			case 'a': { // author
				if (!stream.deserialize<SafeFixedString<SAPPHIRE_LEVEL_AUTHOR_MAX_LEN>>(info.author.getUserName())) {
					return false;
				}
				break;
			}
			case ',': {
				//music in new version
				FixedString musicname;
				if (stream.deserialize<SafeFixedString<SAPPHIRE_MUSIC_NAME_MAX_LEN>>(musicname)) {
					this->musicName = util::move(musicname);
				} else {
					return false;
				}
				break;
			}
			case 'M': { // music
				//before steam release
				uint32 music;
				if (stream.deserialize<uint32>(music)) {
					this->musicName = convertSapphireMusicToMusicName((SapphireMusic) music);
					//return false;
				} else {
					this->musicName = nullptr;
					return false;
				}
				break;
			}
			case 'E': { // max loot lose
				if (!stream.deserialize<int32>(properties.maxLootLose)) {
					return false;
				}
				break;
			}
			case 'p': {
				if (!stream.deserialize<uint32>(properties.pushRate)) {
					return false;
				}
				break;
			}
			case 'd': {
				if (!stream.deserialize<uint32>(properties.dispenserSpeed)) {
					return false;
				}
				break;
			}
			case 's': {
				if (!stream.deserialize<uint32>(properties.swampRate)) {
					return false;
				}
				break;
			}
			case 'o': {
				if (!stream.deserialize<uint32>(properties.robotMoveRate)) {
					return false;
				}
				break;
			}
			case 'w': {
				if (!stream.deserialize<uint32>(properties.wheelTurnTime)) {
					return false;
				}
				break;
			}
			case 'e': {
				if (!stream.deserialize<int32>(properties.loot)) {
					return false;
				}
				break;
			}
			case 'v': {
				if (!stream.deserialize<uint32>(properties.elevatorSpeed)) {
					return false;
				}
				break;
			}
			case 't': {
				if (!stream.deserialize<int32>(properties.maxSteps)) {
					return false;
				}
				break;
			}
			case 'T': {
				if (!stream.deserialize<int32>(properties.maxTime)) {
					return false;
				}
				break;
			}
			case SAPPHIRE_CMD_LEADERBOARDS: {
				if (!stream.deserialize<SapphireLeaderboards>(properties.leaderboards)) {
					return false;
				}
				break;
			}
			case 'y': {
				ASSERT(yamyamRemainders == nullptr) << "Yam yam remainders defined more than once";
				uint32 yamcount;
				if (!stream.deserialize<uint32>(yamcount)) {
					return false;
				}
				if (yamyamRemainderCount > SAPPHIRE_MAX_YAMYAMREMAINDER_COUNT) {
					return false;
				}
				yamyamRemainderCount = yamcount;
				yamyamRemainders = new ObjectIdentifier[yamyamRemainderCount * 9];
				for (unsigned int i = 0; i < yamyamRemainderCount * 9; ++i) {
					if (!readObjectIdentifier(stream, yamyamRemainders + i)) {
						return false;
					}
					countedloot += getLootCountForIdentifier(yamyamRemainders[i]);
				}
				break;
			}
			case 'm': {
				uint32 w;
				uint32 h;
				if (!stream.deserialize<uint32>(w) || !stream.deserialize<uint32>(h)) {
					return false;
				}
				if (w > SAPPHIRE_MAX_LEVEL_DIMENSION || h > SAPPHIRE_MAX_LEVEL_DIMENSION) {
					return false;
				}
				this->width = w;
				this->height = h;
				this->map = new GameObject[width * height];

				for (unsigned int i = 0; i < this->width * this->height; ++i) {
					ObjectIdentifier obj;
					if (!readObjectIdentifier(stream, &obj)) {
						return false;
					}
					GameObject& mo = MAP(i % this->width, this->height - 1 - i / this->width);
					mo.set(getGameObjectForIdentifier(obj));
					switch (mo.object) {
						case SapphireObject::Player:
							++minersTotal;
							if (mo.getPlayerId() == 1)
								playerCount = 2;
							break;
						case SapphireObject::Exit:
							++startexitcount;
							break;
						default:
							countedloot += getLootCount(mo);
							break;
					}
				}
				break;
			}
			case 'R': { //demo
				Demo* d = new Demo();
				if (!stream.deserialize<uint32>(d->randomseed)) {
					return false;
				}
				if (!stream.deserialize<SafeFixedString<SAPPHIRE_LEVEL_TITLE_MAX_LEN>>(d->info.title)) {
					return false;
				}
				if (!stream.deserialize<SafeFixedString<SAPPHIRE_DEMO_MAX_LEN>>(d->moves)) {
					return false;
				}

				addDemo(d);

				break;
			}
			case SAPPHIRE_CMD_END_OF_FILE: {
				goto after_loop;
			}
			default: {
				THROW()<< "unknown map command: " << (unsigned int) cmd;
				break;
			}
		}
	}
	after_loop:

	WARN(this->map == nullptr) << "Map is nullptr";

	if (this->map == nullptr) {
		return false;
	}

	for (unsigned int j = 0; j < height; ++j) {
		for (unsigned int i = 0; i < width; ++i) {
			GameObject& o = MAP(i, j);
			o.x = i;
			o.y = j;
		}
	}

	this->controls = new PlayerControl[this->playerCount];

//	LOGI()<< getInfo().title << " Counted loot: " << countedloot;

	if (properties.loot < 0) {
		targetLoot = countedloot;
	} else {
		targetLoot = properties.loot;
	}
//	LOGI()<< getInfo().title << " Target loot: " << targetLoot;

	this->closeExitOnEnter = startexitcount >= minersTotal;
	this->minersPlaying = minersTotal;
	if (properties.dispenserSpeed > 0) {
		this->currentDispenserValue = properties.dispenserSpeed - 1;
	} else {
		this->currentDispenserValue = 0;
	}
	if (properties.elevatorSpeed > 0) {
		this->currentElevatorValue = properties.elevatorSpeed - 1;
	} else {
		this->currentElevatorValue = 0;
	}

	lastLorrySoundTurn = -1000;
	lastBugSoundTurn = -1000;
	return true;
}
bool Level::loadLevel(FileDescriptor& fd) {
	auto in = BufferedInputStream::wrap(fd.openInputStream());
	return loadLevel(in);
}

bool Level::loadLevel(RAssetFile asset) {
	AssetFileDescriptor fd { asset };
	return loadLevel(fd);
}

void Level::addFallSound(const GameObject& o) {
	switch (o.object) {
		case SapphireObject::Bag: {
			addSound(SapphireSound::FallBag, o);
			break;
		}
		case SapphireObject::Emerald: {
			addSound(SapphireSound::FallEmerald, o);
			break;
		}
		case SapphireObject::Ruby: {
			addSound(SapphireSound::FallRuby, o);
			break;
		}
		case SapphireObject::Sapphire: {
			addSound(SapphireSound::FallSapphire, o);
			break;
		}
		default: {
			break;
		}
	}
}

void Level::addDemo(Demo* demo) {
	demos.add(demo);
}
void Level::removeDemo(unsigned int index) {
	delete demos.remove(index);
}
void Level::removeDemo(const Demo* demo) {
	delete demos.removeOne(demo);
}

void Level::resize(unsigned int width, unsigned int height) {
	auto oldmap = map;
	this->map = new GameObject[width * height];
	if (oldmap != nullptr) {
		for (unsigned int i = 0; i < width && i < this->width; ++i) {
			for (unsigned int j = 0; j < height && j < this->height; ++j) {
				map[j * width + i] = oldmap[j * this->width + i];
			}
		}
		delete[] oldmap;
	}
	this->width = width;
	this->height = height;
	for (unsigned int i = 0; i < width; ++i) {
		for (unsigned int j = 0; j < height; ++j) {
			auto& obj = MAP(i, j);
			obj.x = i;
			obj.y = j;
		}
	}

}
void Level::expand(unsigned int left, unsigned int top, unsigned int right, unsigned int bottom) {
	unsigned int nwidth = width + left + right;
	unsigned int nheight = height + top + bottom;
	auto oldmap = map;
	this->map = new GameObject[nwidth * nheight];
	if (oldmap != nullptr) {
		for (unsigned int i = 0; i < this->width; ++i) {
			for (unsigned int j = 0; j < this->height; ++j) {
				map[(j + bottom) * (nwidth) + left + i] = oldmap[j * this->width + i];
			}
		}
		delete[] oldmap;
	}
	this->width = nwidth;
	this->height = nheight;

	for (unsigned int i = 0; i < nwidth; ++i) {
		for (unsigned int j = 0; j < nheight; ++j) {
			auto& obj = MAP(i, j);
			obj.x = i;
			obj.y = j;
		}
	}
}
void Level::shrink(unsigned int left, unsigned int top, unsigned int right, unsigned int bottom) {
	ASSERT(left + right < this->width) << left << " " << right << " - " << width;
	ASSERT(bottom + top < this->height) << bottom << " " << top << " - " << height;
	unsigned int nwidth = width - left - right;
	unsigned int nheight = height - top - bottom;
	auto oldmap = map;
	this->map = new GameObject[nwidth * nheight];
	if (oldmap != nullptr) {
		for (unsigned int i = 0; i < nwidth; ++i) {
			for (unsigned int j = 0; j < nheight; ++j) {
				map[j * nwidth + i] = oldmap[(j + bottom) * this->width + left + i];
			}
		}
		delete[] oldmap;
	}

	this->width = nwidth;
	this->height = nheight;

	for (unsigned int i = 0; i < nwidth; ++i) {
		for (unsigned int j = 0; j < nheight; ++j) {
			auto& obj = MAP(i, j);
			obj.x = i;
			obj.y = j;
		}
	}
}

void Level::resetState() {
	for (unsigned int i = 0; i < (unsigned int) SapphireSound::_count_of_entries; ++i) {
		soundsIndex[i] = -1;
	}
	soundCount = 0;

	delete[] controls;
	delete[] demoSteps;

	playerCount = 1;

	laserId = 0;

	currentDispenserValue = 0;
	currentElevatorValue = 0;

	wheel = nullptr;

	openedExists = false;
	targetLoot = 0;
	pickedLoot = 0;
	lootLost = 0;

	minersFinished = 0;
	minersTotal = 0;

	minersPlaying = 0;

	currentYamyamRemainder = 0;

	controls = nullptr;

	robotTargets.clear();

	keysCollected.reset();
	bombsCollected.reset();

	demoStepsLength = 64;
	demoSteps = new char[demoStepsLength];

	statistics = LevelStatistics { };

	unsigned int startexitcount = 0;
	unsigned int countedloot = 0;

	for (unsigned int i = 0; i < this->width * this->height; ++i) {
		GameObject& mo = this->map[i];
		switch (mo.object) {
			case SapphireObject::Player:
				++minersTotal;
				if (mo.getPlayerId() == 1) {
					playerCount = 2;
				}
				break;
			case SapphireObject::Exit:
				++startexitcount;
				break;
			default:
				countedloot += getLootCount(mo);
				break;
		}
	}
	for (unsigned int i = 0; i < yamyamRemainderCount * 9; ++i) {
		countedloot += getLootCountForIdentifier(yamyamRemainders[i]);
	}

	this->controls = new PlayerControl[this->playerCount];

	if (properties.loot < 0) {
		targetLoot = countedloot;
	} else {
		targetLoot = properties.loot;
	}

	this->closeExitOnEnter = startexitcount >= minersTotal;
	this->minersPlaying = minersTotal;
	if (properties.dispenserSpeed > 0) {
		this->currentDispenserValue = properties.dispenserSpeed - 1;
	} else {
		this->currentDispenserValue = 0;
	}
	if (properties.elevatorSpeed > 0) {
		this->currentElevatorValue = properties.elevatorSpeed - 1;
	} else {
		this->currentElevatorValue = 0;
	}

	lastLorrySoundTurn = -1000;
	lastBugSoundTurn = -1000;
}

Level::GameObject& Level::setObject(unsigned int x, unsigned int y, SapphireObject obj, SapphireDirection dir) {
	ASSERT(x < width);
	ASSERT(y < height);
	auto& go = MAP(x, y);
	go.set(SampleMap[obj]);
	if (obj == SapphireObject::YamYam || obj == SapphireObject::Bug || obj == SapphireObject::Lorry) {
		ASSERT(dir != SapphireDirection::Undefined);
		go.direction = dir;
	}
	return go;
}
Level::GameObject& Level::setObject(unsigned int x, unsigned int y, const GameObject& proto) {
	ASSERT(x < width);
	ASSERT(y < height);
	auto& go = MAP(x, y);
	go.set(proto);
	return go;
}

Level::GameObject& Level::setObject(unsigned int x, unsigned int y, const ObjectIdentifier& objectchar) {
	ASSERT(x < width);
	ASSERT(y < height);
	auto& go = MAP(x, y);
	go.set(getGameObjectForIdentifier(objectchar));
	return go;
}

void Level::setYamYamRemainderCount(unsigned int count) {
	if (count == yamyamRemainderCount) {
		return;
	}

	const ObjectIdentifier* old = yamyamRemainders;
	if (count != 0) {
		yamyamRemainders = new ObjectIdentifier[count * 9];
		if (old != nullptr) {
			memcpy(yamyamRemainders, old, (count < yamyamRemainderCount ? count : yamyamRemainderCount) * 9 * sizeof(ObjectIdentifier));
			delete[] old;
		}
		if (count > yamyamRemainderCount) {
			//fill with air
			for (int i = yamyamRemainderCount * 9; i < count * 9; ++i) {
				yamyamRemainders[i] = OBJECT_IDENTIFIER_AIR;
			}
		}
	} else {
		yamyamRemainders = nullptr;
		delete[] old;
	}
	yamyamRemainderCount = count;
}

unsigned int Level::getMapPlayerCount() const {
	for (int i = 0; i < getWidth(); ++i) {
		for (int j = 0; j < getHeight(); ++j) {
			auto& o = get(i, j);
			if (o.object == SapphireObject::Player && o.getPlayerId() == 1) {
				return 2;
			}
		}
	}
	return 1;
}

void Level::loseLoot(unsigned int count) {
	int oldlost = lootLost;
	lootLost += count;
	if (oldlost <= properties.maxLootLose && lootLost > properties.maxLootLose && !isSuccessfullyOver()) {
		addSound(SapphireSound::GameLost);
	}
	//LOGI()<<"Loot destructed total: " << lootLost;
}

}
// namespace userapp

