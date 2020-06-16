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
 * Level.h
 *
 *  Created on: 2016. jan. 15.
 *      Author: sipka
 */

#ifndef LEVEL_H_
#define LEVEL_H_

#include <sapphire/level/LevelInfo.h>
#include <sapphire/level/Demo.h>
#include <sapphire/level/SapphireRandom.h>
#include <sapphire/sapphireconstants.h>
#include <sapphire/level/LevelStatistics.h>
#include <framework/utils/ArrayList.h>
#include <framework/utils/LinkedList.h>
#include <framework/geometry/Vector.h>
#include <framework/io/files/FileDescriptor.h>
#include <framework/utils/FixedString.h>

#include <gen/configuration.h>
#include <gen/types.h>
#include <gen/log.h>

namespace userapp {
using namespace rhfw;
enum class SapphireMusic
: uint32;
template<typename T>
class MoveablePointer {
	T* ptr;
public:
	MoveablePointer(T* ptr = nullptr)
			: ptr { ptr } {
	}
	MoveablePointer(const MoveablePointer<T>&) = default;
	MoveablePointer(MoveablePointer<T> && o)
			: ptr { o.ptr } {
		o.ptr = nullptr;
	}
	MoveablePointer<T>& operator=(MoveablePointer<T> && o) {
		this->ptr = o.ptr;
		o.ptr = nullptr;
		return *this;
	}
	MoveablePointer<T>& operator=(const MoveablePointer<T> & o) = default;
	operator T*() const {
		return ptr;
	}
	T& operator[](int index) {
		return ptr[index];
	}
	const T& operator[](int index) const {
		return ptr[index];
	}
	T* operator+(int index) {
		return ptr + index;
	}
	const T* operator+(int index) const {
		return ptr + index;
	}
};

class Level {
public:
	using ObjectIdentifier = unsigned char;
	class GameObject {
	public:
		SapphireVisual visual = SapphireVisual::NO_FLAG;

		SapphireObject object = SapphireObject::Air;
		SapphireDirection direction = SapphireDirection::Undefined;
		SapphireProps props = SapphireProps::Blowable | SapphireProps::Pickable;
		SapphireState state = SapphireState::Still;
		SapphireDynamic dynamic = SapphireDynamic::NO_FLAG;
		unsigned int turn = 0;
		unsigned short x = 0xFFFF;
		unsigned short y = 0xFFFF;

		SapphireExplosion explosionType = SapphireExplosion::None;
		SapphireExplosion propagateExplosion = SapphireExplosion::None;
		SapphireLaser laser = SapphireLaser::NO_FLAG;
		unsigned short laserId = 0;
		ObjectIdentifier explosionResult = ' ';
		unsigned char explosionState = 0;

		GameObject() {
		}
		GameObject(const GameObject& o) = default;
		GameObject& operator=(const GameObject& o) = default;

		void set(const GameObject& o) {
			//skip visual
			this->object = o.object;
			this->direction = o.direction;
			this->props = o.props;
			this->state = o.state;
			this->dynamic = o.dynamic;
			this->turn = o.turn;
			//copy visuals except the laser
			//and except the citrine shattering
			this->visual =
					(o.visual
							& ~(SapphireVisual::MaskLaser | SapphireVisual::PastObjectDirMask | SapphireVisual::PastObjectDirMask
									| SapphireVisual::PastObjectPicked | SapphireVisual::MaskPastObject | SapphireVisual::CitrineShattered))
							| (this->visual
									& (SapphireVisual::MaskLaser | SapphireVisual::PastObjectDirMask | SapphireVisual::PastObjectDirMask
											| SapphireVisual::PastObjectPicked | SapphireVisual::MaskPastObject
											| SapphireVisual::CitrineShattered));
			//skip coordinates

			//skip explosion and laser
		}

		ObjectIdentifier mapToIdentifier() const;

		void setFixedStill() {
			SET_FLAG(dynamic, SapphireDynamic::FixedStill);
		}
		void clearFixedStill() {
			CLEAR_FLAG(dynamic, SapphireDynamic::FixedStill);
		}
		bool isFixedStill() const {
			return HAS_FLAG(dynamic, SapphireDynamic::FixedStill);
		}

		void setJustPushed() {
			SET_FLAG(dynamic, SapphireDynamic::JustPushed);
		}
		void clearJustPushed() {
			CLEAR_FLAG(dynamic, SapphireDynamic::JustPushed);
		}
		bool isJustPushed() const {
			return HAS_FLAG(dynamic, SapphireDynamic::JustPushed);
		}

		void addLaser(SapphireLaser laser) {
			SET_FLAG(this->laser, laser);
		}
		bool hasLaser(SapphireLaser laser) const {
			return HAS_FLAG(this->laser, laser);
		}
		void clearLaser() {
			laserId = 0;
			laser = SapphireLaser::NO_FLAG;
		}

		void setLaserId(unsigned short id) {
			this->laserId = id;
		}
		unsigned short getLaserId() const {
			return laserId;
		}

		void setDefaultTurned() {
			SET_FLAG(dynamic, SapphireDynamic::DefaultTurned);
		}
		void clearDefaultTurned() {
			CLEAR_FLAG(dynamic, SapphireDynamic::DefaultTurned);
		}
		bool isDefaultTurned() const {
			return HAS_FLAG(dynamic, SapphireDynamic::DefaultTurned);
		}

		SapphireDynamic getExitState() const {
			return dynamic & SapphireDynamic::MaskExitState;
		}
		void setExitState(SapphireDynamic state) {
			dynamic = (dynamic & ~SapphireDynamic::MaskExitState) | state;
		}

		unsigned int getPlayerId() const {
			return (unsigned int) ((dynamic & SapphireDynamic::MaskPlayerId) >> SapphireDynamic::PlayerIdShift);
		}
		void setPlayerId(unsigned int id) {
			ASSERT(id <= (unsigned int) SapphireDynamic::MaxPlayerId) << "Invalid player id " << id;
			dynamic = (dynamic & ~SapphireDynamic::MaskPlayerId) | ((SapphireDynamic) id << SapphireDynamic::PlayerIdShift);
		}
		unsigned int getBombCount() const {
			return (unsigned int) ((dynamic & SapphireDynamic::MaskBombCount) >> SapphireDynamic::BombCountShift);
		}
		bool decreaseBombCount() {
			unsigned int count = getBombCount();
			if (count > 0) {
				setBombCount(count - 1);
				return true;
			}
			return false;
		}
		void addBombs(unsigned int count) {
			setBombCount(getBombCount() + count);
		}
		void setBombCount(unsigned int count) {
			//LOGV("Set bomb count: %u", count);
			ASSERT(count <= (unsigned int) SapphireDynamic::MaxBombCount) << "Invalid bomb count " << count;
			dynamic = (dynamic & ~SapphireDynamic::MaskBombCount) | ((SapphireDynamic) count << SapphireDynamic::BombCountShift);
		}

		bool isStoneWallWithObject() const {
			ASSERT(this->object == SapphireObject::StoneWall) << this->object;
			return HAS_FLAG(dynamic, SapphireDynamic::StoneWallWithObject);
		}
		void clearStoneWallObject() {
			CLEAR_FLAG(dynamic, SapphireDynamic::StoneWallWithObject);
		}
		SapphireObject getStoneWallObject() const {
			ASSERT(this->object == SapphireObject::StoneWall) << this->object;
			return (SapphireObject) ((dynamic & SapphireDynamic::StoneWallObjectMask)
					>> (unsigned int) SapphireDynamic::StoneWallObjectShift);
		}
		void setStoneWallObject(SapphireObject obj) {
			ASSERT(this->object == SapphireObject::StoneWall) << this->object;
			SET_FLAG(dynamic, SapphireDynamic::StoneWallWithObject);
			dynamic = (dynamic & ~SapphireDynamic::StoneWallObjectMask) | ((SapphireDynamic) obj << SapphireDynamic::StoneWallObjectShift);
		}

		SapphireObject getDispenserObject() const {
			ASSERT(this->object == SapphireObject::Dispenser) << this->object;
			return (SapphireObject) ((dynamic & SapphireDynamic::DispenserObjectMask)
					>> (unsigned int) SapphireDynamic::DispenserObjectShift);
		}
		void setDispenserObject(SapphireObject obj) {
			ASSERT(this->object == SapphireObject::Dispenser) << this->object;
			dynamic = (dynamic & ~SapphireDynamic::DispenserObjectMask) | ((SapphireDynamic) obj << SapphireDynamic::DispenserObjectShift);
		}

		unsigned int getExplosionState() const {
			return explosionState;
		}
		void increaseExplosionState() {
			++explosionState;
		}
		void startExplosion() {
			//reset state to zero
			explosionState = 0;
		}
		void clearExplosionState() {
			explosionState = 0;
		}
		void setExplosionType(SapphireExplosion type) {
			explosionType = type;
		}
		void startExplosion(const ObjectIdentifier& result, SapphireExplosion type) {
			setExplosionResult(result);
			setExplosionType(type);
			startExplosion();
		}
		void clearExplodePropagate() {
			CLEAR_FLAG(props, SapphireProps::ExplodePropagate);
		}
		bool canExplodePropagate() const {
			return HAS_FLAG(props, SapphireProps::ExplodePropagate);
		}
		void setPropagateExplosion(SapphireExplosion explosion) {
			ASSERT(canExplodePropagate()) << "doesnt have explode propagate flag " << object << " (" << state << ") [" << props << "], "
					<< x << " - " << y;
			//LOGV("Set propagate explosion on %s, %u - %u", TOSTRING(object), x, y);
			propagateExplosion = explosion;
			clearExplodePropagate();
		}
		void clearPropagateExplosion() {
			propagateExplosion = SapphireExplosion::None;
		}
		void clearAnyExplosion() {
			explosionType = SapphireExplosion::None;
		}

		const ObjectIdentifier& getExplosionResult() const {
			return explosionResult;
		}
		void setExplosionResult(const ObjectIdentifier& res) {
			this->explosionResult = res;
		}
		bool isPropagateExplosion() const {
			return propagateExplosion != SapphireExplosion::None;
		}
		SapphireExplosion getPropagateExplosion() const {
			return propagateExplosion;
		}
		bool canFallSideTo() const {
			return (object == SapphireObject::Air) && !isAnyExplosion();
		}
		bool canFallInto() const {
			return (object == SapphireObject::Air || object == SapphireObject::Acid) && !isAnyExplosion();
		}

		bool isAnyExplosion() const {
			return explosionType != SapphireExplosion::None;
		}
		bool isBigExplosion() const {
			return explosionType == SapphireExplosion::Big;
		}
		bool isNormalExplosion() const {
			return explosionType == SapphireExplosion::Normal;
		}

		bool isOneTimeDoorClosed() const {
			return HAS_FLAG(dynamic, SapphireDynamic::DoorClosed);
		}
		void setOneTimeDoorClosed() {
			dynamic |= SapphireDynamic::DoorClosed;
		}
		bool hasRedKey() const {
			return HAS_FLAG(dynamic, SapphireDynamic::KeyRed);
		}
		bool hasGreenKey() const {
			return HAS_FLAG(dynamic, SapphireDynamic::KeyGreen);
		}
		bool hasBlueKey() const {
			return HAS_FLAG(dynamic, SapphireDynamic::KeyBlue);
		}
		bool hasYellowKey() const {
			return HAS_FLAG(dynamic, SapphireDynamic::KeyYellow);
		}
		void addRedKey() {
			dynamic |= SapphireDynamic::KeyRed;
		}
		void addGreenKey() {
			dynamic |= SapphireDynamic::KeyGreen;
		}
		void addBlueKey() {
			dynamic |= SapphireDynamic::KeyBlue;
		}
		void addYellowKey() {
			dynamic |= SapphireDynamic::KeyYellow;
		}

		//Visuals
		void clearVisual() {
			visual = SapphireVisual::NO_FLAG;
		}
		bool isLaser() const {
			return (visual & SapphireVisual::MaskLaser) != 0;
		}
		void setMoving() {
			SET_FLAG(visual, SapphireVisual::Moving);
		}
		bool isMoving() const {
			return HAS_FLAG(visual, SapphireVisual::Moving);
		}
		void setExplosionSpawning() {
			SET_FLAG(visual, SapphireVisual::ExplosionSpawning);
		}
		bool isExplosionSpawning() const {
			return HAS_FLAG(visual, SapphireVisual::ExplosionSpawning);
		}

		bool isCitrineShattered() const {
			return HAS_FLAG(visual, SapphireVisual::CitrineShattered);
		}
		void setCitrineShattered() {
			SET_FLAG(visual, SapphireVisual::CitrineShattered);
		}

		//Past object
		/*void setPastObject(SapphireObject past) {
		 visual = (visual & ~SapphireVisual::MaskPastObject) | ((SapphireVisual) past << SapphireVisual::PastObjectShift);
		 }*/
		SapphireObject getPastObject() const {
			return (SapphireObject) ((visual & SapphireVisual::MaskPastObject) >> SapphireVisual::PastObjectShift);
		}
		void setPastObjectFallInto(SapphireObject object) {
			visual = (visual & ~SapphireVisual::MaskPastObject) | ((SapphireVisual) object << SapphireVisual::PastObjectShift)
					| SapphireVisual::PastObjectFallInto;
		}
		bool isPastObjectFallInto() const {
			return HAS_FLAG(visual, SapphireVisual::PastObjectFallInto);
		}
		void setPastObjectPicked(SapphireObject past, SapphireDirection dir) {
			visual = (visual & (~(SapphireVisual::PastObjectDirMask | SapphireVisual::MaskPastObject)))
					| ((SapphireVisual) dir << SapphireVisual::PastObjectDirShift)
					| ((SapphireVisual) past << SapphireVisual::PastObjectShift) | SapphireVisual::PastObjectPicked;
		}
		bool isPastObjectPicked() const {
			return HAS_FLAG(visual, SapphireVisual::PastObjectPicked);
		}
		SapphireDirection getPastObjectPickDirection() const {
			return (SapphireDirection) ((visual & SapphireVisual::PastObjectDirMask) >> SapphireVisual::PastObjectDirShift);
		}

		//Fallable flags
		bool isRolling() const {
			return HAS_FLAG(visual, SapphireVisual::Rolling);
		}
		void setRolling() {
			SET_FLAG(visual, SapphireVisual::Rolling);
		}

		//Door flags
		void setUsingDoor() {
			SET_FLAG(visual, SapphireVisual::DoorUsing);
		}
		bool isUsingDoor() const {
			return HAS_FLAG(visual, SapphireVisual::DoorUsing);
		}

		//Cushion flags
		void setFallCushion() {
			SET_FLAG(visual, SapphireVisual::FallFinish);
		}
		bool isFallCushion() const {
			return HAS_FLAG(visual, SapphireVisual::FallFinish);
		}

		// Swamp flags
		void setSwampSpawnUp() {
			SET_FLAG(visual, SapphireVisual::SwampUp);
		}
		bool isSwampSpawnUp() const {
			return HAS_FLAG(visual, SapphireVisual::SwampUp);
		}
		void setSwampDropHit() {
			SET_FLAG(visual, SapphireVisual::SwampDropHit);
		}
		bool isSwampDropHit() const {
			return HAS_FLAG(visual, SapphireVisual::SwampDropHit);
		}
		void setSwampHighlight() {
			SET_FLAG(visual, SapphireVisual::SwampHighlight);
		}
		bool isSwampHighlight() const {
			return HAS_FLAG(visual, SapphireVisual::SwampHighlight);
		}

		//Player flags
		void setPlayerDigging() {
			SET_FLAG(visual, SapphireVisual::PlayerDig);
		}
		bool isPlayerDigging() const {
			return HAS_FLAG(visual, SapphireVisual::PlayerDig);
		}
		void setPlayerTryPush() {
			SET_FLAG(visual, SapphireVisual::PlayerTryPush);
		}
		bool isPlayerTryPush() const {
			return HAS_FLAG(visual, SapphireVisual::PlayerTryPush);
		}
		void setPlayerUsingDoor() {
			SET_FLAG(visual, SapphireVisual::PlayerUsingDoor);
		}
		bool isPlayerUsingDoor() const {
			return HAS_FLAG(visual, SapphireVisual::PlayerUsingDoor);
		}
		void setPlayerRobotKill(SapphireDirection dir) {
			visual = (visual & (~(SapphireVisual::PlayerRobotDirMask | SapphireVisual::MaskPastObject)))
					| ((SapphireVisual) dir << SapphireVisual::PlayerRobotDirShift)
					| ((SapphireVisual) SapphireObject::Robot << SapphireVisual::PastObjectShift) | SapphireVisual::PlayerRobotKilled;
		}
		bool isPlayerRobotKilled() const {
			return HAS_FLAG(visual, SapphireVisual::PlayerRobotKilled);
		}
		SapphireDirection getPlayerRobotKillDirection() const {
			return (SapphireDirection) ((visual & SapphireVisual::PlayerRobotDirMask) >> SapphireVisual::PlayerRobotDirShift);
		}
		void setPlayerFacing(SapphireDirection dir) {
			ASSERT(dir != SapphireDirection::Undefined);
			visual = (visual & ~SapphireVisual::PlayerFacingMask) | ((SapphireVisual) dir << SapphireVisual::PlayerFacingShift);
		}
		SapphireDirection getPlayerFacing() const {
			return (SapphireDirection) ((visual & SapphireVisual::PlayerFacingMask) >> SapphireVisual::PlayerFacingShift);
		}

		//YamYam flags
		void setYamYamOldDirection(SapphireDirection dir) {
			visual = (visual & (~SapphireVisual::YamYamOldDirectionMask))
					| ((SapphireVisual) dir << SapphireVisual::YamYamOldDirectionShift) | SapphireVisual::YamYamOldDirectionSet;
		}
		SapphireDirection getYamYamOldDirection() const {
			return HAS_FLAG(visual, SapphireVisual::YamYamOldDirectionSet) ?
					(SapphireDirection) ((visual & SapphireVisual::YamYamOldDirectionMask) >> SapphireVisual::YamYamOldDirectionShift) :
					SapphireDirection::Undefined;
		}
		bool isYamYamEatingGem() const {
			return HAS_FLAG(visual, SapphireVisual::YamYamEatingGem);
		}
		void setYamYamEatingGem() {
			SET_FLAG(visual, SapphireVisual::YamYamEatingGem);
		}

		//TickBomb flags
		void setTickBombSpawning() {
			SET_FLAG(visual, SapphireVisual::TickBombSpawning);
		}
		bool isTickBombSpawning() const {
			return HAS_FLAG(visual, SapphireVisual::TickBombSpawning);
		}

		//Rock flags
		void setSapphireBreaking() {
			SET_FLAG(visual, SapphireVisual::SapphireBreaking);
		}
		bool isSapphireBreaking() const {
			return HAS_FLAG(visual, SapphireVisual::SapphireBreaking);
		}
		void setCitrineBreaking() {
			SET_FLAG(visual, SapphireVisual::CitrineBreaking);
		}
		bool isCitrineBreaking() const {
			return HAS_FLAG(visual, SapphireVisual::CitrineBreaking);
		}

		//Dispenser flags
		void setDispenserSpawn() {
			SET_FLAG(visual, SapphireVisual::DispenserSpawn);
		}
		bool isDispenserSpawn() const {
			return HAS_FLAG(visual, SapphireVisual::DispenserSpawn);
		}
		void setDispenserRecharge() {
			SET_FLAG(visual, SapphireVisual::DispenserRecharge);
		}
		bool isDispenserRecharge() const {
			return HAS_FLAG(visual, SapphireVisual::DispenserRecharge);
		}

		//Exit flags
		void setExitSinkPlayer(unsigned int plrid) {
			SET_FLAG(visual, SapphireVisual::ExitSinkPlayer);
			setPlayerId(plrid);
		}
		bool isExitSinkPlayer() const {
			return HAS_FLAG(visual, SapphireVisual::ExitSinkPlayer);
		}
		void setExitWalkPlayer(unsigned int plrid) {
			SET_FLAG(visual, SapphireVisual::ExitWalkPlayer);
			setPlayerId(plrid);
		}
		bool isExitWalkPlayer() const {
			return HAS_FLAG(visual, SapphireVisual::ExitWalkPlayer);
		}

		//Wheel flags
		void setWheelActive() {
			SET_FLAG(visual, SapphireVisual::WheelActive);
		}
		void clearWheelActive() {
			CLEAR_FLAG(visual, SapphireVisual::WheelActive);
		}
		bool isWheelActive() const {
			return HAS_FLAG(visual, SapphireVisual::WheelActive);
		}
	};

	class GameSound {
	private:
		friend class Level;

		unsigned int count;
		SapphireSound sound;

		unsigned int x;
		unsigned int y;

		//ignore non init warning
		GameSound() {
		}
		GameSound(SapphireSound sound, unsigned int x, unsigned int y)
				: count { 0 }, sound { sound }, x { x }, y { y } {
		}

		void addLocation(unsigned int x, unsigned int y) {
			//TODO
			this->x += x;
			this->y += y;
		}
	public:
		float getX() const {
			return (float) x / count;
		}
		float getY() const {
			return (float) y / count;
		}
		SapphireSound getSound() const {
			return sound;
		}
	};

	class Position: public LinkedNode<Position> {
	public:
		unsigned int x;
		unsigned int y;

		Position(unsigned int x, unsigned int y)
				: x { x }, y { y } {
		}

		Position* get() override {
			return this;
		}
	};

	class LevelProperties {
	public:
		static const int LOOT_DEFAULT = -1;
		static const int MAXLOOTLOSE_DEFAULT = -1;

		static const int MAXSTEPS_DEFAULT = -1;
		static const int MAXTIME_DEFAULT = -1;

		static const unsigned int SWAMPRATE_DEFAULT = 30;
		static const unsigned int PUSHRATE_DEFAULT = 4;
		static const unsigned int ROBOTMOVERATE_DEFAULT = 3;
		static const unsigned int WHEELTURNTIME_DEFAULT = 30;
		static const unsigned int DISPENSERSPEED_DEFAULT = 5;
		static const unsigned int ELEVATORSPEED_DEFAULT = 1;

		static const bool SILENTYAMYAM_DEFAULT = false;
		static const bool SILENTEXPLOSION_DEFAULT = false;

		static const SapphireLeaderboards LEADERBOARDS_DEFAULT = SapphireLeaderboards::NO_FLAG;

		unsigned int swampRate = SWAMPRATE_DEFAULT;
		unsigned int pushRate = PUSHRATE_DEFAULT;
		unsigned int robotMoveRate = ROBOTMOVERATE_DEFAULT;
		unsigned int wheelTurnTime = WHEELTURNTIME_DEFAULT;
		unsigned int dispenserSpeed = DISPENSERSPEED_DEFAULT;
		unsigned int elevatorSpeed = ELEVATORSPEED_DEFAULT;

		bool silentYamYam = SILENTYAMYAM_DEFAULT;
		bool silentExplosion = SILENTEXPLOSION_DEFAULT;

		int maxLootLose = MAXLOOTLOSE_DEFAULT;
		int loot = LOOT_DEFAULT;

		int maxSteps = MAXSTEPS_DEFAULT;
		int maxTime = MAXTIME_DEFAULT;

		SapphireLeaderboards leaderboards = LEADERBOARDS_DEFAULT;

		bool isMaxTimeConstrained() const {
			return maxTime > 0;
		}
		bool isMaxStepConstrained() const {
			return maxSteps > 0;
		}
	};

private:
	class PlayerControl {
	public:
		SapphireDirection dir = SapphireDirection::Undefined;
		SapphireControl control = SapphireControl::Undefined;
	};

	int soundsIndex[(unsigned int) SapphireSound::_count_of_entries];
	GameSound sounds[(unsigned int) SapphireSound::_count_of_entries];
	unsigned int soundCount = 0;

	LevelInfo info;
	unsigned int width = 0;
	unsigned int height = 0;

	unsigned int playerCount = 0;

	unsigned short laserId = 0;

	LevelProperties properties;

	unsigned int currentDispenserValue = 0;
	unsigned int currentElevatorValue = 0;

	bool closeExitOnEnter = true;

	GameObject* wheel = nullptr;
	unsigned int wheelRemainingTurns = 0;

	bool openedExists = false;
	unsigned int targetLoot = 0;
	unsigned int pickedLoot = 0;

	unsigned int minersFinished = 0;
	unsigned int minersTotal = 0;

	unsigned int minersPlaying = 0;

	MoveablePointer<ObjectIdentifier> yamyamRemainders = nullptr;
	unsigned int yamyamRemainderCount = 0;
	unsigned int currentYamyamRemainder = 0;

	ArrayList<Demo> demos;

	int lastLorrySoundTurn = -1000;
	int lastBugSoundTurn = -1000;

	MoveablePointer<PlayerControl> controls = nullptr;

	MoveablePointer<GameObject> map = nullptr;

	MoveablePointer<char> demoSteps = nullptr;
	unsigned int demoStepsLength = 0;

	LinkedList<Position> robotTargets;

	unsigned int originalRandomSeed = 0;
	SapphireRandom random;

	struct KeyCollection {
		SapphireDynamic data[2] { SapphireDynamic::NO_FLAG, SapphireDynamic::NO_FLAG };
		SapphireDynamic& operator[](unsigned int index) {
			return data[index];
		}
		const SapphireDynamic& operator[](unsigned int index) const {
			return data[index];
		}
		void reset() {
			data[0] = SapphireDynamic::NO_FLAG;
			data[1] = SapphireDynamic::NO_FLAG;
		}
	};
	struct BombCollection {
		unsigned int data[2] { 0, 0 };
		unsigned int& operator[](unsigned int index) {
			return data[index];
		}
		const unsigned int& operator[](unsigned int index) const {
			return data[index];
		}
		void reset() {
			data[0] = 0;
			data[1] = 0;
		}
	};
	KeyCollection keysCollected;
	BombCollection bombsCollected;

	FixedString musicName = nullptr;

	int lootLost = 0;

	unsigned int levelVersion = SAPPHIRE_LEVEL_VERSION_NUMBER;

	LevelStatistics statistics;

	void startWheel(GameObject& wheel);

	void triggerFall(GameObject& o);
	void applyExplosionState(Level::GameObject& source, Level::GameObject& expo, SapphireObject sourceobject, bool bigexplosion);
	void applyExplosion(GameObject& source, SapphireExplosion type);

	void redirectStartLaser(GameObject& exploding, GameObject& explosionsource);
	void startLaser(GameObject& ruby, SapphireDirection dir);

	void pickObject(GameObject& picker, GameObject& obj, SapphireDirection dir);

	bool turnExplosion(GameObject& o);

	void randomYamYamDirection(GameObject& yam);
	void moveYamYam(GameObject& yam);
	void moveEnemyBugLorry(GameObject& o, SapphireDirection defaultturn);
	void moveRobot(GameObject& robot);
	bool tryMoveRobot(GameObject& robot, SapphireDirection dir);

	bool checkPlayerEnemyContact(GameObject& player);
	bool checkBugLorrySuicideContact(GameObject& buglorry);
	bool checkYamYamSuicideContact(GameObject& yam);

	void applySwampTurn(GameObject& swamp);
	void applyDispenserTurn(GameObject& dispenser);
	void applyFallableTurn(GameObject& obj);
	void applyPusherObjectTurn(GameObject& obj, SapphireDirection dir);

	bool tryFallTo(Level::GameObject& o, Level::GameObject& below, Level::GameObject& nextto, Level::GameObject& belownextto,
			SapphireDirection dir, Level::GameObject* belowbelow);

	void checkLoot();
	void addLoot(unsigned int count) {
		pickedLoot += count;
	}

	void loseLoot(unsigned int count);

	void setObjectTurn(GameObject& o) {
		o.turn = this->getTurn();
	}

	GameObject* getObjectInDirection(SapphireDirection dir, unsigned int x, unsigned int y);
	GameObject* getObjectInDirection(SapphireDirection dir, const GameObject& src);

	bool canFallDown(GameObject& obj);
	bool canSandSinkablePush(GameObject& obj);

	ObjectIdentifier getBlowResult(SapphireObject sourceobject, const GameObject& exploding, int difx, int dify);

	void actPlayer(GameObject& player);
	Level::GameObject* tryMovePlayerTo(Level::GameObject& player, Level::GameObject& nextto, SapphireDirection dir, bool putbomb);

	bool isExitsOpen() const {
		return pickedLoot >= targetLoot;
	}

	bool collectedEnoughtLoot() const {
		return pickedLoot >= targetLoot;
	}

	void addSound(SapphireSound sound, const GameObject& obj);
	void addSound(SapphireSound sound);

	void addFallSound(const GameObject& obj);

	bool addMoveToPlayer(GameObject& obj);

	unsigned int& turnRef() {
		return statistics.turns;
	}
	unsigned int turnRef() const {
		return statistics.turns;
	}
public:
	Level();
	Level(RAssetFile asset);
	Level(FileDescriptor& fd);
	Level(const Level&);
	Level(Level&&) = default;
	Level& operator=(const Level&);
	Level& operator=(Level&&);
	~Level();

	bool loadLevel(RAssetFile asset);
	bool loadLevel(FileDescriptor& fd);
	bool loadLevel(InputStream& is);
	void saveLevel(FileDescriptor& fd, bool includeuserdemos = false) const;
	void saveLevel(OutputStream& os, bool includeuserdemos = false) const;

	unsigned int getHeight() const {
		return height;
	}
	unsigned int getWidth() const {
		return width;
	}
	Size2UI getSize() const {
		return {width, height};
	}
	unsigned int getPlayerCount() const {
		return playerCount;
	}

	bool hasDemo() const {
		return demos.size() > 0;
	}
	unsigned int getDemoCount() const {
		return demos.size();
	}
	unsigned int getNonUserDemoCount() const {
		unsigned int c = 0;
		for (unsigned int i = 0; i < getDemoCount(); ++i) {
			const Demo* d = getDemo(i);
			if (d->userDemo) {
				continue;
			}
			++c;
		}
		return c;
	}
	const Demo* getDemo(unsigned int index) const {
		return &demos[index];
	}

	bool isGameLost() const {
		return properties.maxLootLose >= 0 && lootLost > properties.maxLootLose;
	}

	void setRandomSeed(unsigned int seed) {
		this->originalRandomSeed = seed;
		this->random.setSeed(seed);
	}
	unsigned int getOriginalRandomSeed() const {
		return originalRandomSeed;
	}

	void applyControl(unsigned int player, SapphireDirection dir, SapphireControl control);

	void clearControls() {
		for (unsigned int i = 0; i < this->playerCount; ++i) {
			controls[i] = PlayerControl { };
		}
	}

	void applyTurn();

	GameObject& get(unsigned int x, unsigned int y) {
		ASSERT(x < width && y < height);
		return map[y * width + x];
	}
	const GameObject& get(unsigned int x, unsigned int y) const {
		ASSERT(x < width && y < height);
		return map[y * width + x];
	}
	const GameObject* getOptional(unsigned int x, unsigned int y) const {
		if (x < width && y < height) {
			return map + (y * width + x);
		}
		return nullptr;
	}
	const GameObject* getOptional(const Size2UI& pos) const {
		return getOptional(pos.x(), pos.y());
	}
	const GameObject& get(const Size2UI& pos) const {
		return get(pos.x(), pos.y());
	}

	bool isOver() const {
		return minersPlaying == 0;
	}
	bool isSuccessfullyOver() const {
		return minersFinished == minersTotal;
	}

	unsigned int getMinersTotal() const {
		return minersTotal;
	}

	unsigned int getMinersFinished() const {
		return minersFinished;
	}

	unsigned int getTurn() const {
		return turnRef();
	}
	const LevelInfo& getInfo() const {
		return info;
	}
	LevelInfo& getInfo() {
		return info;
	}

	unsigned int getSoundCount() const {
		return soundCount;
	}
	const GameSound& getGameSound(unsigned int index) const {
		return sounds[index];
	}

	bool isDispenserTurn() const {
		return currentDispenserValue == properties.dispenserSpeed;
	}
	bool isDispenserRechargeTurn() const {
		return currentDispenserValue == 1;
	}
	unsigned int getDispenserRechargeSpeed() const {
		return properties.dispenserSpeed;
	}
	unsigned int getDispenserValue() const {
		return currentDispenserValue;
	}

	LevelProperties& getProperties() {
		return properties;
	}
	const LevelProperties& getProperties() const {
		return properties;
	}

	bool isKeyPicked(unsigned int playerindex, SapphireDynamic key) const {
		return HAS_FLAG(keysCollected[playerindex], key);
	}

	unsigned int getRemainingLoot() const {
		if (pickedLoot > targetLoot) {
			return 0;
		}
		return targetLoot - pickedLoot;
	}
	unsigned int getCollectedBombCount(unsigned int playerindex) const {
		return bombsCollected[playerindex];
	}

	unsigned int getMoveCount() const {
		return statistics.moveCount;
	}

	const FixedString& getMusicName() const{
		return musicName;
	}
	void setMusicName(const FixedString& name){
		this->musicName = name;
	}

	const char* getRecordedDemo() const {
		return demoSteps;
	}
	unsigned int getRecordedDemoLength() const {
		return this->getTurn() * this->playerCount;
	}
	FixedString getRecordedDemoString() const {
		return FixedString { (const char*) demoSteps, this->getRecordedDemoLength() };
	}

	unsigned int getYamYamRemainderCount() const {
		return yamyamRemainderCount;
	}
	void setYamYamRemainderCount(unsigned int count);
	const ObjectIdentifier* getYamYamRemainders() const {
		return yamyamRemainders;
	}
	ObjectIdentifier* getYamYamRemainders() {
		return yamyamRemainders;
	}

	void addDemo(Demo* demo);
	void removeDemo(unsigned int index);
	void removeDemo(const Demo* demo);

	void resize(unsigned int width, unsigned int height);

	void expand(unsigned int left, unsigned int top, unsigned int right, unsigned int bottom);
	void shrink(unsigned int left, unsigned int top, unsigned int right, unsigned int bottom);

	void resetState();

	GameObject& setObject(unsigned int x, unsigned int y, SapphireObject obj, SapphireDirection dir = SapphireDirection::Undefined);
	GameObject& setObject(unsigned int x, unsigned int y, const GameObject& proto);
	GameObject& setObject(unsigned int x, unsigned int y, const ObjectIdentifier& objectchar);

	unsigned int getMapPlayerCount() const;

	unsigned int getLevelVersion() const {
		return levelVersion;
	}

	void updateLevelVersion() {
		this->levelVersion = SAPPHIRE_LEVEL_VERSION_NUMBER;
	}

	ArrayList<Demo>& getDemos() {
		return demos;
	}
	const ArrayList<Demo>& getDemos() const {
		return demos;
	}

	const LevelStatistics& getStatistics() const {
		return statistics;
	}
};

}

#endif /* LEVEL_H_ */
