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
 * LevelDrawer3D.cpp
 *
 *  Created on: 2016. jul. 21.
 *      Author: sipka
 */

#include <appmain.h>
#include <framework/geometry/Matrix.h>
#include <framework/xml/XmlParser.h>
#include <framework/resource/ResourceManager.h>
#include <framework/utils/ArrayList.h>
#include <framework/utils/utility.h>
#include <sapphire/levelrender/LevelDrawer3D.h>
#include <sapphire/level/Level.h>
#include <sapphire/sapphireconstants.h>
#include <gen/log.h>
#include <gen/xmldecl.h>

#define _USE_MATH_DEFINES
#include <math.h>

using namespace rhfw;
using namespace userapp;

namespace userapp {

inline static float directionToRadian(SapphireDirection dir) {
	switch (dir) {
		case SapphireDirection::Left:
			return 0.0f;
		case SapphireDirection::Up:
			return (float) -M_PI_2;
		case SapphireDirection::Right:
			return (float) M_PI;
		case SapphireDirection::Down:
			return (float) M_PI_2;
		default: {
			THROW()<< "Invalid dir: " << dir;
			return 0.0f;
		}
	}
}
LevelDrawer3D::LevelDrawer3D()
		: level(nullptr), needBackground(false), background(needBackground, 0) {
	drawCommands.addToEnd(defaultCommands);
	drawCommands.addToEnd(dirtCommands);
	drawCommands.addToEnd(dispenserCommands);
	drawCommands.addToEnd(laserCommands);
	drawCommands.addToEnd(minerSinkCommands);
	for (int i = 0; i < 4; ++i) {
		explosioncommands[i].init(objectsBuffer);
		drawCommands.addToEnd(explosioncommands[i]);
	}
	drawCommands.addToEnd(turnAppearingCommands);

	defaultCommands.color = Color { 1, 1, 1, 1 };
}
LevelDrawer3D::LevelDrawer3D(LevelDrawer& parent, const Level* level)
		: level(level), needBackground(parent.isNeedBackground()), background(needBackground, level->getOriginalRandomSeed()) {

	drawCommands.addToEnd(defaultCommands);
	drawCommands.addToEnd(dirtCommands);
	drawCommands.addToEnd(dispenserCommands);
	drawCommands.addToEnd(laserCommands);
	drawCommands.addToEnd(minerSinkCommands);
	for (int i = 0; i < 4; ++i) {
		explosioncommands[i].init(objectsBuffer);
		drawCommands.addToEnd(explosioncommands[i]);
	}
	drawCommands.addToEnd(turnAppearingCommands);

	defaultCommands.color = Color { 1, 1, 1, 1 };

}
LevelDrawer3D::~LevelDrawer3D() {
}

#define MULT_ROLLING(radians) \
	if (o.isRolling()) { \
		if (o.direction == SapphireDirection::Left) { \
			modeltrans = Matrix3D { }.setRotate(-(radians) * turnpercent, 0, 0, 1) * modeltrans; \
			modelinverse.multRotate((radians) * turnpercent, 0, 0, 1); \
		} else { \
			modeltrans = Matrix3D { }.setRotate((radians) * turnpercent, 0, 0, 1) * modeltrans; \
			modelinverse.multRotate(-(radians) * turnpercent, 0, 0, 1); \
		} \
	}

#define DRAW_PAST_OBJECT_FALL_INTO() \
	if (o.isPastObjectFallInto()) { \
		drawObject(getPast3DObject(o.getPastObject()), Matrix3D(exptrans).multTranslate(0, 1 - turnpercent, 0), \
				Matrix3D().setTranslate(0, 1 - turnpercent, 0) *= expinverse); \
	}

template<typename Tester>
void LevelDrawer3D::drawSidedObject(const Level::GameObject& o, Sided3DObject& obj, const Matrix<4>& u_m, const Matrix<4>& u_minv,
		Tester&& test) {
	unsigned int index = 0;
	unsigned int i = o.x;
	unsigned int j = o.y;
	const static unsigned int INDEX_L = 0x1;
	const static unsigned int INDEX_T = 0x2;
	const static unsigned int INDEX_R = 0x4;
	const static unsigned int INDEX_B = 0x8;

	auto* left = level->getOptional(i - 1, j);
	auto* right = level->getOptional(i + 1, j);
	auto* bottom = level->getOptional(i, j - 1);
	auto* top = level->getOptional(i, j + 1);

	if (left == nullptr || !test(*left) || left->isAnyExplosion()) {
		index |= INDEX_L;
	}
	if (top == nullptr || !test(*top) || top->isAnyExplosion()) {
		index |= INDEX_T;
	}
	if (right == nullptr || !test(*right) || right->isAnyExplosion()) {
		index |= INDEX_R;
	}
	if (bottom == nullptr || !test(*bottom) || bottom->isAnyExplosion()) {
		index |= INDEX_B;
	}
	switch (index) {
		case 0: {
			drawObject(obj.covermesh, u_m, u_minv);
			break;
		}
		case INDEX_B: {
			drawObject(obj.bottommesh, u_m, u_minv);
			break;
		}
		case INDEX_L: {
			drawObject(obj.bottommesh, Matrix3D().setRotate(M_PI_2, 0, 0, 1) *= u_m, Matrix3D(u_minv).multRotate(M_PI_2, 0, 0, -1));
			break;
		}
		case INDEX_T: {
			drawObject(obj.bottommesh, Matrix3D().setRotate(M_PI_2 * 2, 0, 0, 1) *= u_m, Matrix3D(u_minv).multRotate(M_PI_2 * 2, 0, 0, -1));
			break;
		}
		case INDEX_R: {
			drawObject(obj.bottommesh, Matrix3D().setRotate(M_PI_2 * 3, 0, 0, 1) *= u_m, Matrix3D(u_minv).multRotate(M_PI_2 * 3, 0, 0, -1));
			break;
		}
		case INDEX_L | INDEX_B: {
			drawObject(obj.leftbottommesh, u_m, u_minv);
			break;
		}
		case INDEX_T | INDEX_L: {
			drawObject(obj.leftbottommesh, Matrix3D().setRotate(M_PI_2, 0, 0, 1) *= u_m, Matrix3D(u_minv).multRotate(M_PI_2, 0, 0, -1));
			break;
		}
		case INDEX_R | INDEX_T: {
			drawObject(obj.leftbottommesh, Matrix3D().setRotate(M_PI_2 * 2, 0, 0, 1) *= u_m,
					Matrix3D(u_minv).multRotate(M_PI_2 * 2, 0, 0, -1));
			break;
		}
		case INDEX_B | INDEX_R: {
			drawObject(obj.leftbottommesh, Matrix3D().setRotate(M_PI_2 * 3, 0, 0, 1) *= u_m,
					Matrix3D(u_minv).multRotate(M_PI_2 * 3, 0, 0, -1));
			break;
		}
		case INDEX_L | INDEX_R: {
			drawObject(obj.leftrightmesh, u_m, u_minv);
			break;
		}
		case INDEX_T | INDEX_B: {
			drawObject(obj.leftrightmesh, Matrix3D().setRotate(M_PI_2, 0, 0, 1) *= u_m, Matrix3D(u_minv).multRotate(M_PI_2, 0, 0, -1));
			break;
		}
		case INDEX_L | INDEX_B | INDEX_R: {
			drawObject(obj.leftbottomrightmesh, u_m, u_minv);
			break;
		}
		case INDEX_L | INDEX_T | INDEX_B: {
			drawObject(obj.leftbottomrightmesh, Matrix3D().setRotate(M_PI_2, 0, 0, 1) *= u_m,
					Matrix3D(u_minv).multRotate(M_PI_2, 0, 0, -1));
			break;
		}
		case INDEX_L | INDEX_R | INDEX_T: {
			drawObject(obj.leftbottomrightmesh, Matrix3D().setRotate(M_PI_2 * 2, 0, 0, 1) *= u_m,
					Matrix3D(u_minv).multRotate(M_PI_2 * 2, 0, 0, -1));
			break;
		}
		case INDEX_R | INDEX_T | INDEX_B: {
			drawObject(obj.leftbottomrightmesh, Matrix3D().setRotate(M_PI_2 * 3, 0, 0, 1) *= u_m,
					Matrix3D(u_minv).multRotate(M_PI_2 * 3, 0, 0, -1));
			break;
		}
		case INDEX_L | INDEX_R | INDEX_T | INDEX_B: {
			drawObject(obj.allmesh, u_m, u_minv);
			break;
		}
		default: {
			THROW()<< "Invalid index " << index;
			break;
		}
	}
}

void LevelDrawer3D::drawExit(float openpercent, const Matrix<4>& u_m, const Matrix<4>& u_minv) {
	drawObject(exit_doorwaymesh, u_m, u_minv);
	drawObject(exit_door_leftmesh, Matrix3D().setRotate(openpercent * M_PI_2, 0, 1, 0).multTranslate(exit_door_leftmesh->getOrigin()) *=
			u_m, Matrix3D(u_minv).multTranslate(-exit_door_leftmesh->getOrigin()).multRotate(openpercent * M_PI_2, 0, -1, 0));
	drawObject(exit_door_rightmesh, Matrix3D().setRotate(openpercent * -M_PI_2, 0, 1, 0).multTranslate(exit_door_rightmesh->getOrigin()) *=
			u_m, Matrix3D(u_minv).multTranslate(-exit_door_rightmesh->getOrigin()).multRotate(openpercent * - M_PI_2, 0, -1, 0));
}
#define DEGTORAD(deg) ((deg) / 180.0f * M_PI)
void LevelDrawer3D::drawYamYam(float openpercent, const Matrix<4>& u_m, const Matrix<4>& u_minv, ColoredCommands& cmd) {
	drawObject(yamyam_headmesh, u_m, u_minv, cmd);
	drawObject(yamyam_mouthinsidemesh, u_m, u_minv, cmd);
	drawObject(yamyam_mouthtopmesh,
			Matrix3D().setRotate(DEGTORAD(openpercent * 12.64f), 1, 0, 0).multTranslate(yamyam_mouthtopmesh->getOrigin()) *= u_m,
			Matrix3D(u_minv).multRotate(DEGTORAD(openpercent * -12.64f), 1, 0, 0).multTranslate(-yamyam_mouthtopmesh->getOrigin()), cmd);
	drawObject(yamyam_teethtopmesh,
			Matrix3D().setRotate(DEGTORAD(openpercent * 20.0f), 1, 0, 0).multTranslate(yamyam_teethtopmesh->getOrigin()) *= u_m,
			Matrix3D(u_minv).multRotate(DEGTORAD(openpercent * -20.0f), 1, 0, 0).multTranslate(-yamyam_teethtopmesh->getOrigin()), cmd);

	drawObject(yamyam_mouthbottommesh,
			Matrix3D().setRotate(DEGTORAD(openpercent * -37.0f), 1, 0, 0).multTranslate(yamyam_mouthbottommesh->getOrigin()) *= u_m,
			Matrix3D(u_minv).multRotate(DEGTORAD(openpercent * 37.0f), 1, 0, 0).multTranslate(-yamyam_mouthtopmesh->getOrigin()), cmd);
	drawObject(yamyam_teethbottommesh,
			Matrix3D().setRotate(DEGTORAD(openpercent * -37.0f), 1, 0, 0).multTranslate(yamyam_teethbottommesh->getOrigin()) *= u_m,
			Matrix3D(u_minv).multRotate(DEGTORAD(openpercent * 37.0f), 1, 0, 0).multTranslate(-yamyam_teethbottommesh->getOrigin()), cmd);

}
void LevelDrawer3D::drawYamYam(float openpercent, float turnpercent, Matrix<4> modeltrans, Matrix<4> modelinverse, SapphireDirection olddir,
		SapphireDirection newdir, ColoredCommands& cmd) {
	static const float TURN_ANGLE = M_PI / 6;
	float oldanglex = 0;
	float oldangley = 0;
	switch (olddir) {
		case SapphireDirection::Left: {
			oldanglex += TURN_ANGLE;
			break;
		}
		case SapphireDirection::Right: {
			oldanglex += -TURN_ANGLE;
			break;
		}
		case SapphireDirection::Up: {
			oldangley += TURN_ANGLE;
			break;
		}
		case SapphireDirection::Down: {
			oldangley += -TURN_ANGLE;
			break;
		}
		default: {
			break;
		}
	}
	float anglex = 0;
	float angley = 0;
	switch (newdir) {
		case SapphireDirection::Left: {
			anglex += TURN_ANGLE;
			break;
		}
		case SapphireDirection::Right: {
			anglex += -TURN_ANGLE;
			break;
		}
		case SapphireDirection::Up: {
			angley += TURN_ANGLE;
			break;
		}
		case SapphireDirection::Down: {
			angley += -TURN_ANGLE;
			break;
		}
		default: {
			break;
		}
	}
	modeltrans = Matrix3D().setRotate(oldanglex + (anglex - oldanglex) * turnpercent, 0, 1, 0) *= modeltrans;
	modelinverse.multRotate(oldanglex + (anglex - oldanglex) * turnpercent, 0, -1, 0);

	modeltrans = Matrix3D().setRotate(oldangley + (angley - oldangley) * turnpercent, 1, 0, 0) *= modeltrans;
	modelinverse.multRotate(oldangley + (angley - oldangley) * turnpercent, -1, 0, 0);
	drawYamYam(openpercent, modeltrans, modelinverse, cmd);
}

void LevelDrawer3D::drawTickBomb(bool bombon, bool tickon, const Matrix<4>& u_m, const Matrix<4>& u_minv, ColoredCommands& cmd) {
	drawObject(tickbombmesh, u_m, u_minv, cmd);
	drawObject(tickon ? tickbomb_led_tickonmesh : tickbomb_led_tickoffmesh, u_m, u_minv, cmd);
	drawObject(bombon ? tickbomb_led_bombonmesh : tickbomb_led_bomboffmesh, u_m, u_minv, cmd);
}

void LevelDrawer3D::drawElevator(bool on, const Matrix<4>& u_m, const Matrix<4>& u_minv) {
	drawObject(elevatormesh, u_m, u_minv);
	drawObject(on ? elevator_ledonmesh : elevator_ledoffmesh, u_m, u_minv);
}

void LevelDrawer3D::drawRobot(float turnpercent, const Matrix<4>& u_m, const Matrix<4>& u_minv, unsigned int levelturn,
		ColoredCommands& cmd) {
	float amount = sinf(turnpercent * M_PI) / 16.0f;
	if (levelturn % 2 == 0) {
		amount = -amount;
	}
	drawObject(robotmesh, Matrix3D().setTranslate(0, 0.5, 0).multShearX(amount).multTranslate(0, -0.5, 0) *= u_m,
			Matrix3D(u_minv).multTranslate(0, 0.5, 0).multShearX(-amount).multTranslate(0, -0.5, 0), cmd);
}
static void rotatePlayerDirection(SapphireDirection direction, float rotatepercent, Matrix<4>& u_m, Matrix<4>& u_minv) {
	switch (direction) {
		case SapphireDirection::Left: {
			u_m = Matrix3D().setRotate(M_PI_2 * rotatepercent, 0, 1, 0) *= u_m;
			u_minv.multRotate(M_PI_2 * rotatepercent, 0, -1, 0);
			break;
		}
		case SapphireDirection::Right: {
			u_m = Matrix3D().setRotate(-M_PI_2 * rotatepercent, 0, 1, 0) *= u_m;
			u_minv.multRotate(-M_PI_2 * rotatepercent, 0, -1, 0);
			break;
		}
		case SapphireDirection::Up: {
			u_m = Matrix3D().setRotate(M_PI * rotatepercent, 0, 1, 0) *= u_m;
			u_minv.multRotate(M_PI * rotatepercent, 0, -1, 0);
			break;
		}
		default: {
			//do not rotate on down and undefined
			break;
		}
	}
}
template<typename ... Args>
void LevelDrawer3D::drawMiner(MinerMesh& miner, float turnpercent, unsigned int levelturn, Matrix<4> u_m, Matrix<4> u_minv,
		SapphireDirection direction, SapphireDirection facing, bool dig, bool move, bool push, Args&&... args) {
	if (direction == SapphireDirection::Undefined) {
		direction = SapphireDirection::Down;
	}
	if (facing == SapphireDirection::Undefined) {
		facing = SapphireDirection::Down;
	}

	int dirdiff = ((unsigned int) direction - (unsigned int) facing) % 4;
	if (dirdiff != 0 && dirdiff != 2 && (!push || direction == SapphireDirection::Down || direction == SapphireDirection::Up)) {
		//do not animate rotation when same direction, 180 degree or (pushing sideways)
		rotatePlayerDirection(facing, 1.0f, u_m, u_minv);
		if (dirdiff == 3) {
			dirdiff = -1;
		}
		float rotatepercent;
		if (turnpercent < 0.4f) {
			rotatepercent = turnpercent / 0.4f;
		} else {
			rotatepercent = 1.0f;
		}
		u_m = Matrix3D().setRotate(M_PI_2 * rotatepercent * dirdiff, 0, 1, 0) *= u_m;
		u_minv.multRotate(M_PI_2 * rotatepercent * dirdiff, 0, -1, 0);
	} else {
		rotatePlayerDirection(direction, 1.0f, u_m, u_minv);
	}

	float turnsin = sinf(turnpercent * M_PI);

	if (push) {
		//rotate body and head
		float bodyangle = direction == SapphireDirection::Up || direction == SapphireDirection::Down ? 0 : M_PI_4;
		const Matrix3D bodyrotate = Matrix3D().setRotate(-bodyangle, 1, 0, 0);
		const Matrix3D bodyrotateinverse = Matrix3D().setRotate(bodyangle, 1, 0, 0);
		float armrotate = direction == SapphireDirection::Up ? M_PI : (direction == SapphireDirection::Down ? 0 : M_PI_2);

		drawObject(miner.headmesh, //
				(Matrix3D(bodyrotateinverse) //
				.multTranslate(miner.headmesh->getOrigin()) //
				.multTranslate(-miner.bodymesh->getOrigin()) //
				*= bodyrotate) //
				.multTranslate(miner.bodymesh->getOrigin()) //
				*= u_m, //

				(Matrix3D(u_minv) //
				.multTranslate(-miner.bodymesh->getOrigin()) //
				*= bodyrotateinverse) //
				.multTranslate(miner.bodymesh->getOrigin()) //
				.multTranslate(-miner.headmesh->getOrigin()) //
				*= bodyrotate, util::forward<Args>(args)...);
		drawObject(miner.bodymesh, //
				Matrix3D(bodyrotate) //
				.multTranslate(miner.bodymesh->getOrigin()) *= u_m,

				Matrix3D(u_minv) //
				.multTranslate(-miner.bodymesh->getOrigin()) //
				*= bodyrotateinverse, util::forward<Args>(args)...);

		drawObject(miner.armleftmesh, //
				(Matrix3D(bodyrotateinverse) //
				.multRotate(armrotate, 1, 0, 0) //
				.multTranslate(miner.armleftmesh->getOrigin()) //
				.multTranslate(-miner.bodymesh->getOrigin()) //
				*= bodyrotate) //
				.multTranslate(miner.bodymesh->getOrigin()) //
				*= u_m, //

				(Matrix3D(u_minv) //
				.multTranslate(-miner.bodymesh->getOrigin()) //
				*= bodyrotateinverse) //
				.multTranslate(miner.bodymesh->getOrigin()) //
				.multTranslate(-miner.armleftmesh->getOrigin()) //
				.multRotate(-armrotate, 1, 0, 0), util::forward<Args>(args)...);
		drawObject(miner.armrightmesh, //
				(Matrix3D(bodyrotateinverse) //
				.multRotate(armrotate, 1, 0, 0) //
				.multTranslate(miner.armrightmesh->getOrigin()) //
				.multTranslate(-miner.bodymesh->getOrigin()) //
				*= bodyrotate) //
				.multTranslate(miner.bodymesh->getOrigin()) //
				*= u_m, //

				(Matrix3D(u_minv) //
				.multTranslate(-miner.bodymesh->getOrigin()) //
				*= bodyrotateinverse) //
				.multTranslate(miner.bodymesh->getOrigin()) //
				.multTranslate(-miner.armrightmesh->getOrigin()) //
				.multRotate(-armrotate, 1, 0, 0), util::forward<Args>(args)...);

		//draw pusing legs
		Sapphire3DObject* frontleg;
		Sapphire3DObject* backleg;
		if (levelturn % 2 == 0) {
			frontleg = miner.legrightmesh;
			backleg = miner.legleftmesh;
		} else {
			frontleg = miner.legleftmesh;
			backleg = miner.legrightmesh;
		}
		if (direction == SapphireDirection::Up || direction == SapphireDirection::Down) {
			drawMinerVerticalWalkingLegs(turnpercent, u_m, u_minv, direction, frontleg, backleg, util::forward<Args>(args)...);
		} else {
			float rotate = turnsin * M_PI_4 / 3 * 2;
			drawObject(backleg, //
					Matrix3D() //
					.setRotate((-M_PI_2 / 8 + -rotate / 3.0f), 1, 0, 0) //
					.multShearZ(0.0f, rotate / 1.5f * 1.2) //
					.multTranslate(0, 0, (-0.03f - turnsin / 20)) //
					.multTranslate(backleg->getOrigin()) *= u_m, //

					Matrix3D(u_minv) //
					.multTranslate(-backleg->getOrigin()) //
					.multTranslate(0, 0, -(-0.03f - turnsin / 20)) //
					.multShearZ(0.0f, -rotate / 1.5f * 1.2) //
					.multRotate(-(-M_PI_2 / 8 + -rotate / 3.0f), 1, 0, 0), util::forward<Args>(args)...);

			drawObject(frontleg, //
					Matrix3D() //
					.setRotate((-M_PI_2 / 8 + rotate / 2 / 3.0f), 1, 0, 0) //
					.multShearZ(0.0f, -rotate / 2 * 1.3) //
					.multTranslate(0, 0, (-0.03f + turnsin / 20)) //
					.multTranslate(frontleg->getOrigin()) *= u_m, //

					Matrix3D(u_minv) //
					.multTranslate(-frontleg->getOrigin()) //
					.multTranslate(0, 0, -(-0.03f + turnsin / 20)) //
					.multShearZ(0.0f, rotate / 2 * 1.3) //
					.multRotate(-(-M_PI_2 / 8 + rotate / 2 / 3.0f), 1, 0, 0), util::forward<Args>(args)...);
		}
	} else {
		drawObjectAtOrigin(miner.headmesh, u_m, u_minv, util::forward<Args>(args)...);
		drawObjectAtOrigin(miner.bodymesh, u_m, u_minv, util::forward<Args>(args)...);
		if (move) {
			//draw walking arms
			//draw walking legs
			Sapphire3DObject* frontleg;
			Sapphire3DObject* backleg;
			Sapphire3DObject* frontarm;
			Sapphire3DObject* backarm;
			if (levelturn % 2 == 0) {
				frontleg = miner.legrightmesh;
				backleg = miner.legleftmesh;
				backarm = miner.armrightmesh;
				frontarm = miner.armleftmesh;
			} else {
				frontleg = miner.legleftmesh;
				backleg = miner.legrightmesh;
				frontarm = miner.armrightmesh;
				backarm = miner.armleftmesh;
			}

			float rotate = turnsin * M_PI_4 / 3 * 2;
			if (direction == SapphireDirection::Up || direction == SapphireDirection::Down) {
				drawMinerVerticalWalkingLegs(turnpercent, u_m, u_minv, direction, frontleg, backleg, util::forward<Args>(args)...);
			} else {
				drawObject(backleg, //
						Matrix3D() //
						.setRotate(-rotate / 3.0f, 1, 0, 0) //
						.multShearZ(0.0f, rotate * 1.15) //
						.multTranslate(0, 0, -turnsin / 20) //
						.multTranslate(backleg->getOrigin()) *= u_m, //

						Matrix3D(u_minv) //
						.multTranslate(-backleg->getOrigin()) //
						.multTranslate(0, 0, turnsin / 20) //
						.multShearZ(0.0f, rotate * -1.15) //
						.multRotate(rotate / 3.0f, 1, 0, 0), util::forward<Args>(args)...);

				drawObject(frontleg, //
						Matrix3D() //
						.setRotate(rotate / 3.0f, 1, 0, 0) //
						.multShearZ(0.0f, -rotate * 1.2) //
						.multTranslate(0, 0, turnsin / 20) //
						.multTranslate(frontleg->getOrigin()) *= u_m, //

						Matrix3D(u_minv) //
						.multTranslate(-frontleg->getOrigin()) //
						.multTranslate(0, 0, -turnsin / 20) //
						.multShearZ(0.0f, rotate * 1.3) //
						.multRotate(-rotate / 3.0f, 1, 0, 0), util::forward<Args>(args)...);
			}

			if (!dig) {
				drawObject(frontarm, //
						Matrix3D() //
						.setRotate(rotate, 1, 0, 0) //
						.multTranslate(frontarm->getOrigin()) *= u_m, //

						Matrix3D(u_minv) //
						.multTranslate(-frontarm->getOrigin()) //
						.multRotate(-rotate, 1, 0, 0), util::forward<Args>(args)...);
				drawObject(backarm, //
						Matrix3D() //
						.setRotate(-rotate, 1, 0, 0) //
						.multTranslate(backarm->getOrigin()) *= u_m, //

						Matrix3D(u_minv) //
						.multTranslate(-backarm->getOrigin()) //
						.multRotate(rotate, 1, 0, 0), util::forward<Args>(args)...);
			}
		} else {
			//draw normal arms
			if (!dig) {
				drawObjectAtOrigin(miner.armleftmesh, u_m, u_minv, util::forward<Args>(args)...);
				drawObjectAtOrigin(miner.armrightmesh, u_m, u_minv, util::forward<Args>(args)...);
			}

			//draw normal legs
			drawObjectAtOrigin(miner.legleftmesh, u_m, u_minv, util::forward<Args>(args)...);
			drawObjectAtOrigin(miner.legrightmesh, u_m, u_minv, util::forward<Args>(args)...);
		}
		if (dig) {
			//draw digging arms
			if (direction == SapphireDirection::Down) {
				const static float ARM_IN_ROTATE = DEGTORAD(22.5f);
				float rotate = DEGTORAD(75) - turnsin * DEGTORAD(75 - 25);
				float trans = 0.05f * turnsin;

				drawObject(miner.armleftmesh, //
						Matrix3D() //
						.setRotate(rotate, 1, 0, 0) //
						.multRotate(-ARM_IN_ROTATE, 0, 1, 0) //
						.multTranslate(0, 0, trans) //
						.multTranslate(miner.armleftmesh->getOrigin()) *= u_m, //

						Matrix3D(u_minv) //
						.multTranslate(-miner.armleftmesh->getOrigin()) //
						.multTranslate(0, 0, -trans) //
						.multRotate(ARM_IN_ROTATE, 0, 1, 0) //
						.multRotate(-rotate, 1, 0, 0), util::forward<Args>(args)...);
				drawObject(miner.armrightmesh, //
						Matrix3D() //
						.setRotate(rotate, 1, 0, 0) //
						.multRotate(ARM_IN_ROTATE, 0, 1, 0) //
						.multTranslate(0, 0, trans) //
						.multTranslate(miner.armrightmesh->getOrigin()) *= u_m, //

						Matrix3D(u_minv) //
						.multTranslate(-miner.armrightmesh->getOrigin()) //
						.multTranslate(0, 0, -trans) //
						.multRotate(-ARM_IN_ROTATE, 0, 1, 0) //
						.multRotate(-rotate, 1, 0, 0), util::forward<Args>(args)...);

				drawObject(miner_drillmesh, //
						Matrix3D() //
						.setRotate(-rotate, 1, 0, 0) //
						.multTranslate(miner_drillmesh->getOrigin()) //
						.multTranslate(-miner.armleftmesh->getOrigin()) //
						.multTranslate(0, 0, trans) //
						.multRotate(rotate, 1, 0, 0) //
						.multTranslate(miner.armleftmesh->getOrigin()) //
						*= u_m, //

						Matrix3D(u_minv) //
						.multTranslate(-miner.armleftmesh->getOrigin()) //
						.multRotate(-rotate, 1, 0, 0) //
						.multTranslate(0, 0, -trans) //
						.multTranslate(miner.armleftmesh->getOrigin()) //
						.multTranslate(-miner_drillmesh->getOrigin()) //
						.multRotate(rotate, 1, 0, 0), util::forward<Args>(args)...);
			} else {
				float rotate = DEGTORAD(135) - turnsin * DEGTORAD(135 - 30);
				float trans = 0.05f * turnsin;
				const static float ARM_IN_ROTATE = DEGTORAD(30);
				drawObject(miner.armleftmesh, //
						Matrix3D() //
						.setRotate(rotate, 1, 0, 0) //
						.multRotate(-ARM_IN_ROTATE, 0, 1, 0) //
						.multTranslate(0, 0, trans) //
						.multTranslate(miner.armleftmesh->getOrigin()) *= u_m, //

						Matrix3D(u_minv) //
						.multTranslate(-miner.armleftmesh->getOrigin()) //
						.multTranslate(0, 0, -trans) //
						.multRotate(ARM_IN_ROTATE, 0, 1, 0) //
						.multRotate(-rotate, 1, 0, 0), util::forward<Args>(args)...);
				drawObject(miner.armrightmesh, //
						Matrix3D() //
						.setRotate(rotate, 1, 0, 0) //
						.multRotate(ARM_IN_ROTATE, 0, 1, 0) //
						.multTranslate(0, 0, trans) //
						.multTranslate(miner.armrightmesh->getOrigin()) *= u_m, //

						Matrix3D(u_minv) //
						.multTranslate(-miner.armrightmesh->getOrigin()) //
						.multTranslate(0, 0, -trans) //
						.multRotate(-ARM_IN_ROTATE, 0, 1, 0) //
						.multRotate(-rotate, 1, 0, 0), util::forward<Args>(args)...);

				float costp = fabsf(cosf(turnpercent * M_PI));
				drawObject(miner_pickaxemesh, //
						Matrix3D() //
						.setRotate(DEGTORAD(-20) + costp * DEGTORAD(25), 1, 0, 0) //
						.multTranslate(miner_pickaxemesh->getOrigin()) //
						.multTranslate(-miner.armleftmesh->getOrigin()) //
						.multRotate(rotate, 1, 0, 0) //
						.multTranslate(miner.armleftmesh->getOrigin()) //
						*= u_m, //

						Matrix3D(u_minv) //
						.multTranslate(-miner.armleftmesh->getOrigin()) //
						.multRotate(-rotate, 1, 0, 0) //
						.multTranslate(miner.armleftmesh->getOrigin()) //
						.multTranslate(-miner_pickaxemesh->getOrigin()) //
						.multRotate(-(DEGTORAD(-20) + costp * DEGTORAD(25)), 1, 0, 0), util::forward<Args>(args)...);
			}
		}
	}

}
template<typename ... Args>
void LevelDrawer3D::drawMinerVerticalWalkingLegs(float turnpercent, Matrix<4> u_m, Matrix<4> u_minv, SapphireDirection direction,
		Sapphire3DObject* frontleg, Sapphire3DObject* backleg, Args&&... args) {
	float turnsin = sinf(turnpercent * M_PI);
	float rotate = turnsin * M_PI_4 / 3 * 2;

	drawObject(backleg, //
			Matrix3D() //
			.setRotate(-rotate / 6.0f, 1, 0, 0) //
			.multTranslate(0, -turnsin / 30, -turnsin / 30) //
			.multTranslate(backleg->getOrigin()) *= u_m, //

			Matrix3D(u_minv) //
			.multTranslate(-backleg->getOrigin()) //
			.multTranslate(0, turnsin / 30, turnsin / 30) //
			.multRotate(rotate / 6.0f, 1, 0, 0), util::forward<Args>(args)...);

	drawObject(frontleg, //
			Matrix3D() //
			.setRotate(rotate / 6.0f, 1, 0, 0) //
			.multTranslate(0, turnsin / 30, turnsin / 30) //
			.multTranslate(frontleg->getOrigin()) *= u_m, //

			Matrix3D(u_minv) //
			.multTranslate(-frontleg->getOrigin()) //
			.multTranslate(0, -turnsin / 30, -turnsin / 30) //
			.multRotate(-rotate / 6.0f, 1, 0, 0), util::forward<Args>(args)...);
}

void LevelDrawer3D::drawDoor(float turnpercent, Sapphire3DObject* cover, Sapphire3DObject* left, Sapphire3DObject* right, Matrix<4> u_m,
		Matrix<4> u_minv, const Level::GameObject& o) {
	//front left back right
	Matrix3D models[4];
	Matrix3D minverse[4];
	for (int i = 0; i < 4; ++i) {
		models[i].setIdentity();
		minverse[i].setIdentity();
	}
	if (o.isUsingDoor()) {
		float turnsin = sinf(turnpercent * M_PI) * 0.95f;
		float scale = 1.0f + turnsin / 4.0f;
		u_m = Matrix3D().setScale(scale, scale, scale) *= u_m;
		u_minv.multScale(1 / scale, 1 / scale, 1 / scale);

		float inangle = turnsin * M_PI_2;
		float outangle = turnsin * M_PI_2;

		const int i = (int) o.x;
		const int j = (int) o.y;
		if (j + 1 < level->getHeight() && level->get(i, j + 1).object == SapphireObject::Player
				&& level->get(i, j + 1).direction == SapphireDirection::Up) {
			models[2].setRotate(outangle, 0, 0, 1);
			minverse[2].setRotate(outangle, 0, 0, -1);
			models[0].setRotate(inangle, 0, 0, -1);
			minverse[0].setRotate(inangle, 0, 0, 1);
		}
		if (i + 1 < level->getWidth() && level->get(i + 1, j).object == SapphireObject::Player
				&& level->get(i + 1, j).direction == SapphireDirection::Right) {
			models[1].setRotate(outangle, 0, 0, 1);
			minverse[1].setRotate(outangle, 0, 0, -1);
			models[3].setRotate(inangle, 0, 0, -1);
			minverse[3].setRotate(inangle, 0, 0, 1);
		}
		if (j - 1 >= 0 && level->get(i, j - 1).object == SapphireObject::Player
				&& level->get(i, j - 1).direction == SapphireDirection::Down) {
			models[0].setRotate(outangle, 0, 0, 1);
			minverse[0].setRotate(outangle, 0, 0, -1);
			models[2].setRotate(inangle, 0, 0, -1);
			minverse[2].setRotate(inangle, 0, 0, 1);
		}
		if (i - 1 >= 0 && level->get(i - 1, j).object == SapphireObject::Player
				&& level->get(i - 1, j).direction == SapphireDirection::Left) {
			models[3].setRotate(outangle, 0, 0, 1);
			minverse[3].setRotate(outangle, 0, 0, -1);
			models[1].setRotate(inangle, 0, 0, -1);
			minverse[1].setRotate(inangle, 0, 0, 1);
		}
	}

	//draw cover
	drawObject(cover, u_m, u_minv);

	for (int i = 0; i < 4; ++i) {
		drawObject(left, //
				Matrix3D(models[i]) //
				.multTranslate(left->getOrigin()) //
				.multRotate(-M_PI_2 * i, 0, 0, 1) //
				*= u_m, //

				Matrix3D(u_minv) //
				.multRotate(M_PI_2 * i, 0, 0, 1) //
				.multTranslate(-left->getOrigin()) //
				*= minverse[i] //
						);
		drawObject(right, //
				Matrix3D(minverse[i]) //
				.multTranslate(right->getOrigin()) //
				.multRotate(-M_PI_2 * i, 0, 0, 1) //
				*= u_m, //

				Matrix3D(u_minv) //
				.multRotate(M_PI_2 * i, 0, 0, 1) //
				.multTranslate(-right->getOrigin()) //
				*= models[i] //
						);
	}

	if (o.object == SapphireObject::DoorOneTime) {
		if (o.isUsingDoor()) {
			drawObject(door_onetime_lockmesh, Matrix3D { u_m }.multTranslate(0, 0, -0.05 * (1 - turnpercent)),
					Matrix3D().setTranslate(0, 0, 0.05 * (1 - turnpercent)) *= u_minv, turnAppearingCommands);
		} else {
			if (o.isOneTimeDoorClosed()) {
				drawObject(door_onetime_lockmesh, u_m, u_minv);
			}
		}
	}
}

void LevelDrawer3D::drawAcid(float turnpercent, const Level::GameObject& o, const Matrix<4>& u_m, const Matrix<4>& u_minv) {
	Sapphire3DObject* wavemesh = (o.x) % 2 == 1 ? acid_waves_leftmesh : acid_waves_rightmesh;

	auto* left = level->getOptional(o.x - 1, o.y);
	auto* right = level->getOptional(o.x + 1, o.y);

	float percent = (((level->getTurn() + (o.x) / 2) % 4) + turnpercent) / 4.0f;
	float sinpercent = sinf(percent * M_PI * 2);
	drawObject(acid_bodymesh, u_m, u_minv);
	drawObject(wavemesh, //
			Matrix3D() //
			.setScale(1, sinpercent + 1.2f, 1) //
			.multTranslate(wavemesh->getOrigin()) *= u_m, //

			Matrix3D(u_minv) //
			.multTranslate(-wavemesh->getOrigin()) //
			.multScale(1, 1 / (sinpercent + 1.2f), 1));

	float backpercent = (((level->getTurn() + (o.x + 1) / 2) % 4) + turnpercent) / 4.0f + 0.3333f;
	float sinbackpercent = sinf(backpercent * M_PI * 2);
	drawObject(wavemesh, //
			Matrix3D() //
			.setRotate(M_PI, 0, 1, 0) //
			.multScale(1, sinbackpercent + 1.2f, 1) //
			.multTranslate(wavemesh->getOrigin()) *= u_m, //

			Matrix3D(u_minv) //
			.multTranslate(-wavemesh->getOrigin()) //
			.multScale(1, 1 / (sinbackpercent + 1.2f), 1) //
			.multRotate(M_PI, 0, -1, 0));

	if (left == nullptr || left->object != SapphireObject::Acid || left->isAnyExplosion()) {
		drawObject(acid_sidemesh, u_m, u_minv);
	}
	if (right == nullptr || right->object != SapphireObject::Acid || right->isAnyExplosion()) {
		drawObject(acid_sidemesh, Matrix3D().setRotate(M_PI, 0, 1, 0) *= u_m, Matrix3D(u_minv).multRotate(M_PI, 0, -1, 0));
	}
}

void LevelDrawer3D::drawSand(const Level::GameObject& o, const Matrix<4>& u_m, const Matrix<4>& u_minv) {
	auto* top = level->getOptional(o.x, o.y + 1);
	auto* bot = level->getOptional(o.x, o.y - 1);
	auto* left = level->getOptional(o.x - 1, o.y);
	auto* right = level->getOptional(o.x + 1, o.y);

	drawObject(sand2mesh, u_m, u_minv);

	if (top == nullptr || (top->object != SapphireObject::Sand && top->object != SapphireObject::SandRock) || top->isAnyExplosion()) {
		float angle = (o.x + o.y) * M_PI_2;
		drawObject(sand2_topmesh, Matrix3D().setRotate(angle, 0, 1, 0) *= u_m, Matrix3D(u_minv).multRotate(angle, 0, -1, 0));
	}
	if (bot == nullptr
			|| (bot->object != SapphireObject::Sand && bot->object != SapphireObject::SandRock && bot->object != SapphireObject::Dispenser)
			|| bot->isAnyExplosion()) {
		drawObject(sand2_botmesh, u_m, u_minv);
	}
	if (left == nullptr
			|| (left->object != SapphireObject::Sand && left->object != SapphireObject::SandRock
					&& left->object != SapphireObject::Dispenser) || left->isAnyExplosion()) {
		drawObject(sand2mesh, Matrix3D().setRotate(M_PI_2, 0, 1, 0) *= u_m, Matrix3D(u_minv).multRotate(M_PI_2, 0, -1, 0));
	}
	if (right == nullptr
			|| (right->object != SapphireObject::Sand && right->object != SapphireObject::SandRock
					&& right->object != SapphireObject::Dispenser) || right->isAnyExplosion()) {
		drawObject(sand2mesh, Matrix3D().setRotate(M_PI_2, 0, -1, 0) *= u_m, Matrix3D(u_minv).multRotate(M_PI_2, 0, 1, 0));
	}
}

void LevelDrawer3D::drawConverter(float turnpercent, const Level::GameObject& o, const Matrix<4>& u_m, const Matrix<4>& u_minv) {
	drawObject(converter_basemesh, Matrix3D().setRotate((o.x + o.y) % 4 * M_PI_2, 0, 1, 0) *= u_m,
			Matrix3D(u_minv).multRotate((o.x + o.y) % 4 * M_PI_2, 0, -1, 0));

	float rotatetop = ((level->getTurn() + o.x + o.y) % 4 + turnpercent) / 4.0f * M_PI * 2;
	float rotatemid = ((level->getTurn() + o.x + o.y) % 16 + turnpercent) / 16.0f * M_PI * 2;
	float rotatebot = ((level->getTurn() + o.x + o.y) % 8 + turnpercent) / 8.0f * -M_PI * 2;

	float lighttoprot = (int) ((level->getTurn() + o.x + o.y) % 4 + turnpercent * 4) * DEGTORAD(131);
	float lightbotrot = (int) ((level->getTurn() + o.x + o.y) % 4 + turnpercent * 4) * DEGTORAD(337);

	drawObject(converter_lightningmesh, Matrix3D().setRotate(lighttoprot, 0, 1, 0) *= u_m,
			Matrix3D(u_minv).multRotate(lighttoprot, 0, -1, 0));
	drawObject(converter_lightningmesh, Matrix3D().setRotate(lightbotrot, 0, 1, 0).multRotate(M_PI, 0, 0, 1) *= u_m,
			Matrix3D(u_minv).multRotate(M_PI, 0, 0, -1).multRotate(lightbotrot, 0, -1, 0));

	drawObject(converter_ringmesh, Matrix3D().setRotate(rotatemid, 0, 1, 0) *= u_m, Matrix3D(u_minv).multRotate(rotatemid, 0, -1, 0));
	drawObject(converter_ringmesh, //
			Matrix3D() //
			.setTranslate(converter_ringmesh->getOrigin()) //
			.multRotate(rotatetop, 0, 1, 0) *= u_m, //
			Matrix3D(u_minv) //
			.multRotate(rotatetop, 0, -1, 0) //
			.multTranslate(-converter_ringmesh->getOrigin()) //
					);
	drawObject(converter_ringmesh, //
			Matrix3D() //
			.setTranslate(-converter_ringmesh->getOrigin()) //
			.multRotate(rotatebot, 0, 1, 0) *= u_m, //
			Matrix3D(u_minv) //
			.multRotate(rotatebot, 0, -1, 0) //
			.multTranslate(converter_ringmesh->getOrigin()) //
					);
}

void LevelDrawer3D::drawBug(float turnpercent, const Matrix<4>& u_m, const Matrix<4>& u_minv) {
	drawObject(bug_bodymesh, u_m, u_minv);
	drawBugLegs(turnpercent, u_m, u_minv, bug_leg_general_rightmesh, 1.0f);
	drawBugLegs(turnpercent, u_m, u_minv, bug_leg_general_leftmesh, -1.0f);
}

void LevelDrawer3D::drawBugLegs(float turnpercent, Matrix<4> u_m, Matrix<4> u_minv, Sapphire3DObject* leg, float anglemult) {
	u_m = Matrix3D().setTranslate(leg->getOrigin()) *= u_m;
	u_minv.multTranslate(-leg->getOrigin());

	const float percent = sinf(turnpercent * M_PI);

	drawObject(leg, //
			Matrix3D() //
			.setRotate(anglemult * (-DEGTORAD(45) * percent - DEGTORAD(20)), 0, 0, 1) //
			*= u_m, //

			Matrix3D(u_minv) //
			.multRotate(anglemult * (-DEGTORAD(45) * percent - DEGTORAD(20)), 0, 0, -1) //
					);

	drawObject(leg, //
			Matrix3D() //
			.setRotate(anglemult * DEGTORAD(50) * percent, 0, 0, 1) //
			.multTranslate(0.2f, 0, 0) //
			*= u_m, //

			Matrix3D(u_minv) //
			.multRotate(anglemult * DEGTORAD(50) * percent, 0, 0, -1) //
			.multTranslate(-0.2f, 0, 0) //
					);

	drawObject(leg, //
			Matrix3D() //
			.setRotate(anglemult * (-DEGTORAD(55) * percent + DEGTORAD(70)), 0, 0, 1) //
			.multTranslate(0.43f, 0, 0) //
			*= u_m, //

			Matrix3D(u_minv) //
			.multRotate(anglemult * (-DEGTORAD(55) * percent + DEGTORAD(70)), 0, 0, -1) //
			.multTranslate(-0.43f, 0, 0) //
					);
}

template<typename Handler>
static void iterateRangeOrdered(int start, int end, int mid, Handler&& h) {
	if (start == end) {
		return;
	}
	ASSERT(end >= start) << start << " - " << end;
	ASSERT(mid >= start && mid <= end) << start << " - " << mid << " - " << end;

	util::forward<Handler>(h)(mid);

	//optimize out ifs inside loops, create 2 parts
	const int leftmax = mid - start;
	const int rightmax = end - mid - 1;

	const int withoutif = min(leftmax, rightmax);
	for (int i = 1; i <= withoutif; ++i) {
		const int left = mid - i;
		const int right = mid + i;
		util::forward<Handler>(h)(left);
		util::forward<Handler>(h)(right);
	}
	if (leftmax > rightmax) {
		for (int i = withoutif + 1; i <= leftmax; ++i) {
			const int left = mid - i;
			util::forward<Handler>(h)(left);
		}
	} else {
		for (int i = withoutif + 1; i <= rightmax; ++i) {
			const int right = mid + i;
			util::forward<Handler>(h)(right);
		}
	}
}

void LevelDrawer3D::draw(LevelDrawer& parent, float turnpercent, float alpha, const Size2UI& begin, const Size2UI& end, const Vector2F& mid,
		const Size2F& objectSize) {
	draw(turnpercent, alpha, begin, end, mid, objectSize, parent.isDispenserFullOpacity(), parent.getSize().pixelSize,
			parent.getPaddings());
}
void LevelDrawer3D::draw(float turnpercent, float alpha, const Size2UI& begin, const Size2UI& end, const Vector2F& mid,
		const Size2F& objectSize, bool fulldispenser, const Size2UI& pixelsize, const Rectangle& paddings) {
	ASSERT(end.x() > begin.x());
	ASSERT(end.y() > begin.y());

	unsigned int midx;
	if (mid.x() <= begin.x()) {
		midx = begin.x();
	} else {
		midx = (unsigned int) (mid.x());
		if (midx >= end.x()) {
			midx = end.x() - 1;
		}
	}
	unsigned int midy;
	if (mid.y() <= begin.y()) {
		midy = begin.y();
	} else {
		midy = (unsigned int) (mid.y());
		if (midy >= end.y()) {
			midy = end.y() - 1;
		}
	}

	drawSingle(level->get(midx, midy), turnpercent);

	auto levelsize = level->getSize();
	for (int r = 1; true; ++r) {
		const int ib = ((int) midx) - r;
		const int ie = ((int) midx) + r;

		const int jb = ((int) midy) - r;
		const int je = ((int) midy) + r;

		if (ib < (int) begin.x() && ie >= (int) end.x() && jb < (int) begin.y() && je >= (int) end.y()) {
			break;
		}

		int xstart = max(ib + 1, (int) begin.x());
		int xend = min(ie, (int) end.x());

		int ystart = max(jb + 1, (int) begin.y());
		int yend = min(je, (int) end.y());

		if (jb < levelsize.height()) {
			if (je < levelsize.height()) {
				iterateRangeOrdered(xstart, xend, midx, [&](int i) {
					drawSingle(level->get(i, jb), turnpercent);
					drawSingle(level->get(i, je), turnpercent);
				});
			} else {
				iterateRangeOrdered(xstart, xend, midx, [&](int i) {
					drawSingle(level->get(i, jb), turnpercent);
				});
			}
		} else {
			if (je < levelsize.height()) {
				iterateRangeOrdered(xstart, xend, midx, [&](int i) {
					drawSingle(level->get(i, je), turnpercent);
				});
			}
		}
		if (ib < levelsize.width()) {
			if (ie < levelsize.width()) {
				iterateRangeOrdered(ystart, yend, midy, [&](int j) {
					drawSingle(level->get(ib, j), turnpercent);
					drawSingle(level->get(ie, j), turnpercent);
				});
			} else {
				iterateRangeOrdered(ystart, yend, midy, [&](int j) {
					drawSingle(level->get(ib, j), turnpercent);
				});
			}
		} else {
			if (ie < levelsize.width()) {
				iterateRangeOrdered(ystart, yend, midy, [&](int j) {
					drawSingle(level->get(ie, j), turnpercent);
				});
			}
		}

		if (auto* obj = level->getOptional(ib, jb)) {
			drawSingle(*obj, turnpercent);
		}
		if (auto* obj = level->getOptional(ie, jb)) {
			drawSingle(*obj, turnpercent);
		}
		if (auto* obj = level->getOptional(ib, je)) {
			drawSingle(*obj, turnpercent);
		}
		if (auto* obj = level->getOptional(ie, je)) {
			drawSingle(*obj, turnpercent);
		}
	}

	Size2F fieldsize = pixelsize / objectSize;
	const float depth = 5.0f + min(fieldsize.length(), ((Size2F) end - (Size2F) begin).length()) / 2.0f;

	float dispenseralpha;
	if (fulldispenser) {
		dispenseralpha = 1.0f;
	} else {
		ASSERT(level != nullptr);
		unsigned int dispenserspeed = level->getDispenserRechargeSpeed();
		dispenseralpha = dispenserspeed == 0 ? 1.0f : ((level->getDispenserValue() % dispenserspeed) + turnpercent) / dispenserspeed;
	}
	dispenserCommands.color = Color { 1, 1, 1, dispenseralpha };

	finishDrawing(alpha, turnpercent, objectSize, pixelsize, paddings, depth, mid);

}
void LevelDrawer3D::finishDrawing(float alpha, float turnpercent, const Size2F& objectSize, const Size2UI& pixelsize,
		const Rectangle& paddings, float depth, const Vector2F& mid) {

	turnAppearingCommands.color = Color { 1, 1, 1, turnpercent };

	minerSinkCommands.color = Color { 1, 1, 1, (1 - turnpercent) };

	float laseralphaval;
	if (turnpercent <= 0.25f) {
		laseralphaval = 1.0f - turnpercent / 0.25f;
		laseralphaval = 1.0f - laseralphaval * laseralphaval;
	} else {
		laseralphaval = 1.0f - (turnpercent - 0.25f) / 0.75f;
	}
	laserCommands.color = Color { 1, 1, 1, laseralphaval };

	dirtCommands.color = Color { 1, 1, 1, 1 - turnpercent * turnpercent };

	for (int i = 0; i < 4; ++i) {
		float exppercent = i / 4.0f + turnpercent / 4.0f;
		if (exppercent <= 0.25f) {
			float scale = 1.0f - exppercent / 0.25f;
			scale = 1.0f - scale * scale;
			explosioncommands[i].color = Color { scale, scale, 0, scale };
		} else if (exppercent <= 0.75f) {
			exppercent -= 0.25f;
			explosioncommands[i].color = Color { 1.0f - exppercent, 1.0f - exppercent / 0.5f * 0.75f, 0, 1.0f };
		} else {
			float scale = 1 - ((exppercent - 0.75f) / 0.25f);
			explosioncommands[i].color = Color { scale * 0.5f, scale * 0.25f, 0, scale };
		}
	}

	Size2F fieldsize = pixelsize / objectSize;

	Matrix3D projtrans;
	if (orthographic) {
		projtrans.setOrthographic(renderer, fieldsize.width() / -2.0f, fieldsize.width() / 2.0f, fieldsize.height() / -2.0f,
				fieldsize.height() / 2.0f, (depth - SAPPHIRE_LEVEL3D_DEPTH_THRESHOLD), (depth + SAPPHIRE_LEVEL3D_DEPTH_THRESHOLD));
	} else {
		projtrans.setFrustum(renderer, fieldsize.width() / -2.0f, fieldsize.height() / 2.0f, fieldsize.width() / 2.0f,
				fieldsize.height() / -2.0f, depth - SAPPHIRE_LEVEL3D_DEPTH_THRESHOLD, depth + SAPPHIRE_LEVEL3D_DEPTH_THRESHOLD);
	}

	Matrix3D viewtrans;
	Matrix3D viewinverse;
	viewtrans.setTranslate(-mid.x(), -mid.y(), 0.0f);
	viewtrans.multTranslate(0, 0, -depth);

	viewinverse.setTranslate(0, 0, depth);
	viewinverse.multTranslate(mid.x(), mid.y(), 0.0f);

	const Color ambientlighting { alpha, alpha, alpha, 1.0f };
	const Vector4F lightposition { lightingFixed ? lightingPosition : (mid + 0.5f), 3.0f + depth * 0.1f, 1.0f };

	Color alphamult { 1, 1, 1, alpha };

	int vertexcount = 0;

	renderer->setFaceCulling(true);
	renderer->setDepthTest(true);
	renderer->initDraw();
	renderer->clearDepthBuffer();
	renderer->setTopology(Topology::TRIANGLES);

	if (background.isEnabled()) {
		ASSERT(level != nullptr);

		background.activateInputLayout();

		sapphirePhongShader->useProgram();
		colorStateUniform->update(
				{ viewtrans, viewinverse.transposed(), projtrans, Vector4F { lightingFixed ? lightingPosition : (mid + 0.5f), 3.0f, 1.0f } });
		colorAmbientLighting->update( { ambientlighting });

		colorMaterialUniform->update( { Color { SAPPHIRE_BACKGROUND_DIFFUSE_GRAY_COLOR, SAPPHIRE_BACKGROUND_DIFFUSE_GRAY_COLOR,
		SAPPHIRE_BACKGROUND_DIFFUSE_GRAY_COLOR, 1.0f } * alphamult, Color { 0.00f, 0.00f, 0.00f, 1.0f } * alphamult, Color { 0.00f, 0.00f,
				0.00f, 1.0f } * alphamult.a(), 0.0f });
		sapphirePhongShader->set(colorMaterialUniform);
		sapphirePhongShader->set(colorStateUniform);
		sapphirePhongShader->set(colorAmbientLighting);

		Size2F paddingsize = pixelsize - paddings.leftTop() - paddings.rightBottom();
		Size2F paddedfieldsize = paddingsize / objectSize;

		Matrix3D backmvp;
		Matrix3D backmvpinverse;
		//* 1.1f to correct the scale down because of perspective projection
		float backgroundscale = (max(fieldsize.width(), fieldsize.height()) + 2) * 1.1f;
		if (level->getWidth() <= paddedfieldsize.width() && level->getHeight() <= paddedfieldsize.height()) {
			Vector2F translate = mid - backgroundscale / 2.0f;
			//will not be scrolling
			backmvp.setIdentity().multScale(backgroundscale, backgroundscale, 1).multTranslate(translate.x(), translate.y(), 0);
			backmvpinverse.setIdentity().multTranslate(-translate.x(), -translate.y(), 0).multScale(1.0f / backgroundscale,
					1.0f / backgroundscale, 1.0f);

			colorShaderUniform->update( { backmvp, backmvpinverse.transposed() });
			sapphirePhongShader->set(colorShaderUniform);
			sapphirePhongShader->draw(0, background.getTriangleCount() * 3);
		} else {
			//will be scrolling
			Size2F paddingfieldsize = (paddings.leftTop() + paddings.rightBottom()) / objectSize;
			float paddingmax = max(paddingfieldsize.width(), paddingfieldsize.height());
			Vector2F levelpad { (paddedfieldsize - Vector2F { level->getSize() }) / 2.0f };
			if (levelpad.x() < 0.0f) {
				levelpad.x() = 0.0f;
			}
			if (levelpad.y() < 0.0f) {
				levelpad.y() = 0.0f;
			}

			Vector2F translate { -paddingfieldsize - 1 - levelpad };

			Vector2I offset { (mid - translate - (fieldsize + 2) / 2.0f) / backgroundscale };
			if (offset.x() < 0) {
				offset.x() = 0;
			}
			if (offset.y() < 0) {
				offset.y() = 0;
			}

			//draw 4
			for (unsigned int i = 0; i < 2; ++i) {
				for (unsigned int j = 0; j < 2; ++j) {
					backmvp.setScale(backgroundscale, backgroundscale, 1).multTranslate(translate.x() + (i + offset.x()) * backgroundscale,
							translate.y() + (j + offset.y()) * backgroundscale, 0);
					backmvpinverse.setTranslate(-(translate.x() + (i + offset.x()) * backgroundscale),
							-(translate.y() + (j + offset.y()) * backgroundscale), 0).multScale(1.0f / backgroundscale,
							1.0f / backgroundscale, 1.0f);

					colorShaderUniform->update( { backmvp, backmvpinverse.transposed() });
					sapphirePhongShader->set(colorShaderUniform);
					sapphirePhongShader->draw(0, background.getTriangleCount() * 3);
				}
			}

		}

		renderer->clearDepthBuffer();
		//scale with -1 because of face culling
		//do not clear the depth buffer here, just draw the rectangle far back enough
#ifndef SAPPHIRE_SCREENSHOT_MODE
		drawRectangleColor(
				Matrix3D().setScale(1, -1, 1).multTranslate(0, level->getHeight(), -SAPPHIRE_LEVEL3D_DEPTH_THRESHOLD * 0.95f) *= viewtrans
						* projtrans, Color { 0, 0, 0, 0.2f } * alphamult,
				Rectangle { 0, 0, (float) level->getWidth(), (float) level->getHeight() });
#endif /* !defined(SAPPHIRE_SCREENSHOT_MODE) */
		renderer->setTopology(Topology::TRIANGLES);
	}

	objectsBuffer->getColoredInputLayout()->activate();
	sapphirePhongShader->useProgram();
	colorStateUniform->update( { viewtrans, viewinverse.transposed(), projtrans, lightposition });
	colorAmbientLighting->update( { ambientlighting });
	sapphirePhongShader->set(colorStateUniform);
	sapphirePhongShader->set(colorAmbientLighting);
	for (auto&& cmdnode : drawCommands.nodes()) {
		auto& cmds = *static_cast<ColoredCommands*>(cmdnode);
		unsigned int colormatcount = objectsBuffer->getColoredMaterialCount();
		for (unsigned int i = 0; i < colormatcount; ++i) {
			auto& list = cmds.commands.coloreds[i];
			if (list.isEmpty()) {
				continue;
			}
			auto& material = objectsBuffer->getColoredMaterial(i);
			colorMaterialUniform->update(
					{ material.diffuseColor * cmds.color * alphamult, material.ambient * cmds.color * alphamult, material.specular
							* cmds.color.a() * alphamult.a(), material.specularExponent });
			sapphirePhongShader->set(colorMaterialUniform);
			for (auto&& cmdnode : list.nodes()) {
				auto* cmd = static_cast<DrawCommand*>(cmdnode);
				colorShaderUniform->update( { cmd->mvp, cmd->mvpInverseTranspose });
				sapphirePhongShader->set(colorShaderUniform);
				sapphirePhongShader->draw(cmd->vertexIndex, cmd->vertexCount);
				vertexcount += cmd->vertexCount;
			}
			drawCommandCache.takeAllEnd(list);
		}
	}
	objectsBuffer->getTexturedInputLayout()->activate();
	sapphireTexturedPhongShader->useProgram();
	textureStateUniform->update( { viewtrans, viewinverse.transposed(), projtrans, lightposition });
	textureAmbientLighting->update( { ambientlighting });
	sapphireTexturedPhongShader->set(textureStateUniform);
	sapphireTexturedPhongShader->set(textureAmbientLighting);
	for (auto&& cmdnode : drawCommands.nodes()) {
		auto& cmds = *static_cast<ColoredCommands*>(cmdnode);
		unsigned int texturedmatcount = objectsBuffer->getTexturedMaterialCount();
		for (unsigned int i = 0; i < texturedmatcount; ++i) {
			auto& list = cmds.commands.textureds[i];
			if (list.isEmpty()) {
				continue;
			}
			auto& material = objectsBuffer->getTexturedMaterial(i);
			textureMaterialUniform->update( { cmds.color * alphamult, material.ambient, material.specular, material.specularExponent,
					(render::Texture*) material.diffuseColorMap });
			sapphireTexturedPhongShader->set(textureMaterialUniform);
			for (auto&& cmdnode : list.nodes()) {
				auto* cmd = static_cast<DrawCommand*>(cmdnode);
				textureShaderUniform->update( { cmd->mvp, cmd->mvpInverseTranspose });
				sapphireTexturedPhongShader->set(textureShaderUniform);
				sapphireTexturedPhongShader->draw(cmd->vertexIndex, cmd->vertexCount);
				vertexcount += cmd->vertexCount;
			}
			drawCommandCache.takeAllEnd(list);
		}
	}

	//LOGI()<< "Triangles: " << vertexcount / 3;
}
void LevelDrawer3D::keyObjectTransform(Matrix3D& u_m, Matrix3D& u_minv, float turnpercent, unsigned int turn) {
	u_m = Matrix3D { }.setRotate((turn % 4) * M_PI_2 + turnpercent * M_PI_2, 0.2, 1, 0) * u_m;
	u_minv.multRotate(-((turn % 4) * M_PI_2 + turnpercent * M_PI_2), 0.2, 1, 0);
}
void LevelDrawer3D::drawKey(float turnpercent, Sapphire3DObject* mesh, Matrix<4> u_m, Matrix<4> u_minv, unsigned int turn) {
	keyObjectTransform(u_m, u_minv, turnpercent, turn);
	drawObject(mesh, u_m, u_minv);
}
Sapphire3DObject* LevelDrawer3D::getPast3DObject(SapphireObject object, Matrix3D& u_m, Matrix3D& u_minv, float turnpercent) {
	switch (object) {
		case SapphireObject::Bag: {
			return bagmesh;
		}
		case SapphireObject::Bomb: {
			return bombmesh;
		}
		case SapphireObject::Emerald: {
			return emeraldmesh;
		}
		case SapphireObject::Ruby: {
			return rubymesh;
		}
		case SapphireObject::Sapphire: {
			return sapphiremesh;
		}
		case SapphireObject::Citrine: {
			return citrinemesh;
		}
		case SapphireObject::Rock: {
			return rockmesh;
		}
		case SapphireObject::Drop: {
			return dropmesh;
		}
		case SapphireObject::Robot: {
			return robotmesh;
		}
		case SapphireObject::KeyRed: {
			keyObjectTransform(u_m, u_minv, turnpercent, level->getTurn());
			return key_redmesh;
		}
		case SapphireObject::KeyGreen: {
			keyObjectTransform(u_m, u_minv, turnpercent, level->getTurn());
			return key_greenmesh;
		}
		case SapphireObject::KeyBlue: {
			keyObjectTransform(u_m, u_minv, turnpercent, level->getTurn());
			return key_bluemesh;
		}
		case SapphireObject::KeyYellow: {
			keyObjectTransform(u_m, u_minv, turnpercent, level->getTurn());
			return key_yellowmesh;
		}
		default: {
			THROW()<< "Unknown past object " << object;
			break;
		}
	}
	return nullptr;
}
Sapphire3DObject* LevelDrawer3D::getPast3DObject(SapphireObject object) {
	Matrix3D m;
	Matrix3D u_m;
	return getPast3DObject(object, m, u_m, 0.0f);
}

bool Sapphire3DObject::load() {
	AssetFileDescriptor desc { ResourceManager::idToFile(resourceId) };
	auto stream = desc.openInputStream();

	stream.deserialize<Vector3F>(origin);
	stream.deserialize<uint32>(coloredCount);
	stream.deserialize<uint32>(texturedCount);

	//LOGTRACE() << resourceId << " coloredCount: " << coloredCount;
	//LOGTRACE() << resourceId << " texturedCount: " << texturedCount;

	if (coloredCount > 0) {
		coloredRanges = new DrawingRange[coloredCount];
		for (uint32 i = 0; i < coloredCount; ++i) {
			auto& dr = coloredRanges[i];
			stream.deserialize<uint32>(dr.materialIndex);
			stream.deserialize<uint32>(dr.vertexIndex);
			stream.deserialize<uint32>(dr.vertexCount);
		}
	}
	if (texturedCount > 0) {
		texturedRanges = new DrawingRange[texturedCount];
		for (uint32 i = 0; i < texturedCount; ++i) {
			auto& dr = texturedRanges[i];
			stream.deserialize<uint32>(dr.materialIndex);
			stream.deserialize<uint32>(dr.vertexIndex);
			stream.deserialize<uint32>(dr.vertexCount);
		}
	}

	return true;
}

void Sapphire3DObject::free() {
	delete[] coloredRanges;
	delete[] texturedRanges;
}

bool SapphireObjectBuffer::load() {
	auto assetid = ResourceManager::idToFile(resourceId);
	AssetFileDescriptor desc { assetid };
	auto stream = desc.openInputStream();

	uint32 coloredtriangles;
	uint32 texturedtriangles;
	stream.deserialize<uint32>(coloredtriangles);
	stream.deserialize<uint32>(texturedtriangles);
	stream.deserialize<uint32>(colorMaterialCount);
	stream.deserialize<uint32>(textureMaterialCount);

	LOGTRACE()<< "coloredtriangles: " << coloredtriangles;
	LOGTRACE()<< "texturedtriangles: " << texturedtriangles;
	LOGTRACE()<< "colorMaterialCount: " << colorMaterialCount;
	LOGTRACE()<< "textureMaterialCount: " << textureMaterialCount;

	if (coloredtriangles > 0) {
		colorMaterials = new ColoredMaterial[colorMaterialCount];
		coloredBuffer = renderer->createVertexBuffer();
		coloredInputLayout = Resource<SapphirePhongShader::InputLayout> { sapphirePhongShader->createInputLayout(),
				[&](SapphirePhongShader::InputLayout* il) {
					il->setLayout<SapphirePhongShader::VertexInput>(coloredBuffer);
				} };

		for (int i = 0; i < colorMaterialCount; ++i) {
			ColoredMaterial& mat = colorMaterials[i];
			stream.deserialize<Color>(mat.diffuseColor);
			stream.deserialize<Color>(mat.ambient);
			stream.deserialize<Color>(mat.specular);
			stream.deserialize<float>(mat.specularExponent);
		}

		long long pos = stream.getPosition();
		stream.skip((sizeof(Vector3F) * 2) * coloredtriangles * 3);
		coloredBuffer->setBufferInitializer<SapphirePhongShader::VertexInput>(
				[assetid, pos, coloredtriangles](SapphirePhongShader::VertexInput* initer) {
					AssetFileDescriptor desc {assetid};
					auto stream = desc.openInputStream();
					stream.seek(pos, SeekMethod::BEGIN);
					for (int i = 0; i < coloredtriangles * 3; ++i) {
						Vector3F pos;
						Vector3F normal;

						stream.deserialize<Vector3F>(pos);
						stream.deserialize<Vector3F>(normal);

						initer[i] = {Vector4F {pos, 1.0f}, Vector4F {normal, 0.0f}};
					}
				}, coloredtriangles * 3);
		coloredBuffer->initialize(BufferType::IMMUTABLE);
	}
	if (texturedtriangles > 0) {
		textureMaterials = new TexturedMaterial[textureMaterialCount];
		texturedBuffer = renderer->createVertexBuffer();
		texturedInputLayout = Resource<SapphireTexturedPhongShader::InputLayout> { sapphireTexturedPhongShader->createInputLayout(),
				[&](SapphireTexturedPhongShader::InputLayout* il) {
					il->setLayout<SapphireTexturedPhongShader::VertexInput>(texturedBuffer);
				} };

		for (int i = 0; i < textureMaterialCount; ++i) {
			TexturedMaterial& mat = textureMaterials[i];
			ResId textureres;

			stream.deserialize<ResId>(textureres);
			stream.deserialize<Color>(mat.ambient);
			stream.deserialize<Color>(mat.specular);
			stream.deserialize<float>(mat.specularExponent);

			mat.diffuseColorMap = getTexture(textureres);
		}

		long long pos = stream.getPosition();
		stream.skip((sizeof(Vector3F) * 2 + sizeof(Vector2F)) * texturedtriangles * 3);
		texturedBuffer->setBufferInitializer<SapphireTexturedPhongShader::VertexInput>(
				[assetid, pos, texturedtriangles](SapphireTexturedPhongShader::VertexInput* initer) {
					AssetFileDescriptor desc {assetid};
					auto stream = desc.openInputStream();
					stream.seek(pos, SeekMethod::BEGIN);
					for (int i = 0; i < texturedtriangles * 3; ++i) {
						Vector3F pos;
						Vector3F normal;
						Vector2F texcoord;

						stream.deserialize<Vector3F>(pos);
						stream.deserialize<Vector3F>(normal);
						stream.deserialize<Vector2F>(texcoord);

						initer[i] = {Vector4F {pos, 1.0f}, texcoord, Vector4F {normal, 0.0f}};
					}
				}, texturedtriangles * 3);
		texturedBuffer->initialize(BufferType::IMMUTABLE);
	}

	return true;
}

void SapphireObjectBuffer::free() {
	coloredBuffer = nullptr;
	coloredInputLayout = nullptr;
	texturedBuffer = nullptr;
	texturedInputLayout = nullptr;
	delete[] colorMaterials;
	delete[] textureMaterials;
}

void LevelDrawer3D::ColoredCommands::add(Sapphire3DObject* mesh, const Matrix<4>& u_m, Matrix<4> u_minv, LinkedList<DrawCommand>& cache) {
	u_minv.transpose();
	if (mesh->getColoredCount() > 0) {
		for (unsigned int i = 0; i < mesh->getColoredCount(); ++i) {
			auto& part = mesh->getColored(i);

			DrawCommand* dcmd;
			if (cache.isEmpty()) {
				dcmd = new DrawCommand();
			} else {
				dcmd = static_cast<DrawCommand*>(cache.first());
				dcmd->removeLinkFromList();
			}

			dcmd->mvp = u_m;
			dcmd->mvpInverseTranspose = u_minv;
			dcmd->vertexIndex = part.vertexIndex;
			dcmd->vertexCount = part.vertexCount;

			commands.coloreds[part.materialIndex].addToEnd(*dcmd);
		}
	}
	if (mesh->getTexturedCount() > 0) {
		for (unsigned int i = 0; i < mesh->getTexturedCount(); ++i) {
			auto& part = mesh->getTextured(i);

			DrawCommand* dcmd;
			if (cache.isEmpty()) {
				dcmd = new DrawCommand();
			} else {
				dcmd = static_cast<DrawCommand*>(cache.first());
				dcmd->removeLinkFromList();
			}

			dcmd->mvp = u_m;
			dcmd->mvpInverseTranspose = u_minv;
			dcmd->vertexIndex = part.vertexIndex;
			dcmd->vertexCount = part.vertexCount;

			commands.textureds[part.materialIndex].addToEnd(*dcmd);
		}
	}
}

void LevelDrawer3D::ColoredCommands::addBuilding(Sapphire3DObject* mesh, const Matrix<4>& u_m, Matrix<4> u_minv, float percent,
		LinkedList<DrawCommand>& cache) {
	ASSERT(percent >= 0.0f && percent <= 1.0f) << percent;

	u_minv.transpose();
	if (mesh->getColoredCount() > 0) {
		for (unsigned int i = 0; i < mesh->getColoredCount(); ++i) {
			auto& part = mesh->getColored(i);

			DrawCommand* dcmd;
			if (cache.isEmpty()) {
				dcmd = new DrawCommand();
			} else {
				dcmd = static_cast<DrawCommand*>(cache.first());
				dcmd->removeLinkFromList();
			}

			dcmd->mvp = u_m;
			dcmd->mvpInverseTranspose = u_minv;
			dcmd->vertexIndex = part.vertexIndex;
			dcmd->vertexCount = part.vertexCount * percent;
			dcmd->vertexCount -= dcmd->vertexCount % 3;

			commands.coloreds[part.materialIndex].addToEnd(*dcmd);
		}
	}
	if (mesh->getTexturedCount() > 0) {
		for (unsigned int i = 0; i < mesh->getTexturedCount(); ++i) {
			auto& part = mesh->getTextured(i);

			DrawCommand* dcmd;
			if (cache.isEmpty()) {
				dcmd = new DrawCommand();
			} else {
				dcmd = static_cast<DrawCommand*>(cache.first());
				dcmd->removeLinkFromList();
			}

			dcmd->mvp = u_m;
			dcmd->mvpInverseTranspose = u_minv;
			dcmd->vertexIndex = part.vertexIndex;
			dcmd->vertexCount = part.vertexCount * percent;
			dcmd->vertexCount -= dcmd->vertexCount % 3;

			commands.textureds[part.materialIndex].addToEnd(*dcmd);
		}
	}
}
static Vector3F directionToVector(SapphireDirection dir) {
	switch (dir) {
		case SapphireDirection::Down:
			return Vector3F { 0, -1, 0 };
		case SapphireDirection::Up:
			return Vector3F { 0, 1, 0 };
		case SapphireDirection::Left:
			return Vector3F { -1, 0, 0 };
		case SapphireDirection::Right:
			return Vector3F { 1, 0, 0 };
		default:
			return Vector3F{0,0,0};
	}
}

static void multMatrixTranslateToDirection(Matrix3D& mat, Matrix3D& matinv, SapphireDirection dir, float turnpercent = 1.0f) {
	switch (dir) {
		case SapphireDirection::Down:
			mat.multTranslate(0.0f, 1 - turnpercent, 0.0f);
			matinv = Matrix3D { }.setTranslate(0.0f, -(1 - turnpercent), 0.0f) *= matinv;
			break;
		case SapphireDirection::Up:
			mat.multTranslate(0.0f, -1 + turnpercent, 0.0f);
			matinv = Matrix3D { }.setTranslate(0.0f, -(-1 + turnpercent), 0.0f) *= matinv;
			break;
		case SapphireDirection::Left:
			mat.multTranslate(1 - turnpercent, 0.0f, 0.0f);
			matinv = Matrix3D { }.setTranslate(-(1 - turnpercent), 0.0f, 0.0f) *= matinv;
			break;
		case SapphireDirection::Right:
			mat.multTranslate(-1 + turnpercent, 0.0f, 0.0f);
			matinv = Matrix3D { }.setTranslate(-(-1 + turnpercent), 0.0f, 0.0f) *= matinv;
			break;
		default:
			break;
	}
}
void LevelDrawer3D::drawFallingDrop(float turnpercent, const Matrix<4>& modeltrans, const Matrix<4>& modelinverse, unsigned int mod,
		ColoredCommands& cmd) {
	float amount = sinf(turnpercent * M_PI) / 20.0f;
	if (mod % 2 == 0) {
		amount = -amount;
	}
	drawObject(dropmesh, Matrix3D().setScale(1 + amount, 1 - amount, 1 + amount) *= modeltrans,
			Matrix3D(modelinverse).multScale(1 / (1 + amount), 1 / (1 - amount), 1 / (1 + amount)), cmd);
}
void LevelDrawer3D::drawStandingMiner(MinerMesh& minermesh, const Matrix<4>& modeltrans, const Matrix<4>& modelinverse,
		ColoredCommands& cmd) {
	drawMiner(minermesh, 0.0f, 0, modeltrans, modelinverse, SapphireDirection::Undefined, SapphireDirection::Undefined, false, false, false,
			cmd);
}
void LevelDrawer3D::drawWalkingMiner(MinerMesh& minermesh, float turnpercent, const Matrix<4>& modeltrans, const Matrix<4>& modelinverse,
		SapphireDirection dir, SapphireDirection facing, unsigned int mod, ColoredCommands& cmd) {
	drawMiner(minermesh, turnpercent, mod, modeltrans, modelinverse, dir, facing, false, true, false, cmd);
}
void LevelDrawer3D::drawSingle(const Level::GameObject& o, float turnpercent) {
	int i = o.x;
	int j = o.y;

	Matrix3D modeltrans;
	Matrix3D modelinverse;
	modeltrans.setIdentity();
	modelinverse.setIdentity();

	switch (o.object) {
		case SapphireObject::Bug: {
			if (o.state == SapphireState::Still && !o.isAnyExplosion()) {
				modeltrans.multRotate((1 - turnpercent) * (float) (o.isDefaultTurned() ? M_PI_2 : -M_PI_2), 0, 0, -1);
				modelinverse = Matrix3D { }.setRotate(-(1 - turnpercent) * (float) (o.isDefaultTurned() ? M_PI_2 : -M_PI_2), 0, 0, -1) *=
						modelinverse;
			}
			modeltrans.multRotate(directionToRadian(o.direction), 0, 0, -1);
			modelinverse = Matrix3D { }.setRotate(-directionToRadian(o.direction), 0, 0, -1) *= modelinverse;
			break;
		}
		case SapphireObject::Lorry: {
			if (o.state == SapphireState::Still && !o.isAnyExplosion()) {
				modeltrans.multRotate((1 - turnpercent) * (float) (o.isDefaultTurned() ? -M_PI_2 : M_PI_2), 0, 0, -1);
				modelinverse = Matrix3D { }.setRotate(-(1 - turnpercent) * (float) (o.isDefaultTurned() ? -M_PI_2 : M_PI_2), 0, 0, -1) *=
						modelinverse;
			}
			modeltrans.multRotate(directionToRadian(o.direction), 0, 0, -1);
			modelinverse = Matrix3D { }.setRotate(-directionToRadian(o.direction), 0, 0, -1) *= modelinverse;
			break;
		}
		case SapphireObject::Wheel: {
			if (o.isWheelActive()) {
				modeltrans.multRotate(turnpercent * (float) M_PI_2, 0, 0, -1);
				modelinverse = Matrix3D { }.setRotate(-turnpercent * (float) M_PI_2, 0, 0, -1) *= modelinverse;
			}
			break;
		}
		default: {
			break;
		}
	}

	if (o.isMoving()) {
		switch (o.direction) {
			case SapphireDirection::Down:
				if (HAS_FLAG(o.props, SapphireProps::Fallable)) {
					const float percent = sinf(turnpercent * M_PI);
					if (o.object != SapphireObject::Drop) {
						const static float ROTATION_OFFSET = M_PI / 32;
						if (((i + j) % 2) == 0) {
							modeltrans.multRotate(percent * ROTATION_OFFSET, 0, 1, 1);
							modelinverse = Matrix3D { }.setRotate(-percent * ROTATION_OFFSET, 0, 1, 1) *= modelinverse;
						} else {
							modeltrans.multRotate(-percent * ROTATION_OFFSET, 0, 1, 1);
							modelinverse = Matrix3D { }.setRotate(percent * ROTATION_OFFSET, 0, 1, 1) *= modelinverse;
						}
					}
				}
				modeltrans.multTranslate(0.0f, 1 - turnpercent, 0.0f);
				modelinverse = Matrix3D { }.setTranslate(0.0f, -(1 - turnpercent), 0.0f) *= modelinverse;
				break;
			case SapphireDirection::Up:
				modeltrans.multTranslate(0.0f, -1 + turnpercent, 0.0f);
				modelinverse = Matrix3D { }.setTranslate(0.0f, -(-1 + turnpercent), 0.0f) *= modelinverse;
				break;
			case SapphireDirection::Left:
				modeltrans.multTranslate(1 - turnpercent, 0.0f, 0.0f);
				modelinverse = Matrix3D { }.setTranslate(-(1 - turnpercent), 0.0f, 0.0f) *= modelinverse;
				break;
			case SapphireDirection::Right:
				modeltrans.multTranslate(-1 + turnpercent, 0.0f, 0.0f);
				modelinverse = Matrix3D { }.setTranslate(-(-1 + turnpercent), 0.0f, 0.0f) *= modelinverse;
				break;
			default:
				break;
		}
	}
	modeltrans.multTranslate(0.5f + i, 0.5f + j, 0);
	modelinverse = Matrix3D { }.setTranslate(-(0.5f + i), -(0.5f + j), 0) *= modelinverse;

	const Matrix3D exptrans = Matrix3D().setTranslate(0.5f + i, 0.5f + j, 0);
	const Matrix3D expinverse = Matrix3D().setTranslate(-(0.5f + i), -(0.5f + j), 0);

	if (o.isAnyExplosion() || o.isPropagateExplosion()) {
		float exppercent = o.getExplosionState() / 4.0f + turnpercent / 4.0f;
		float scale;
		if (exppercent <= 0.25f) {
			scale = 1.0f - exppercent / 0.25f;
			scale = 1.0f - scale * scale;
		} else if (exppercent <= 0.75f) {
			scale = 1.0f;
		} else {
			scale = 1 - ((exppercent - 0.75f) / 0.25f);
		}
		Matrix3D exprot;
		Matrix3D exprotinverse = expinverse;

		float coeff = (float) ((i + 17) * (j + 13));
		float sincoeff = sinf(coeff);
		float coscoeff = cosf(coeff);

		Vector3F rotaxes { sincoeff, coscoeff, sincoeff + coscoeff };
		Vector3F angles { exppercent / 2, exppercent / 2, exppercent / 2 };

		exprot.setScale(scale, scale, scale);
		exprot.multRotate(angles.x() * M_PI, rotaxes.x(), rotaxes.y(), 0);
		exprot.multRotate(angles.y() * M_PI, 0, rotaxes.y(), rotaxes.z());
		exprot.multRotate(angles.z() * M_PI, rotaxes.x(), 0, rotaxes.z());

		exprotinverse.multRotate(angles.z() * -M_PI, rotaxes.x(), 0, rotaxes.z());
		exprotinverse.multRotate(angles.y() * -M_PI, 0, rotaxes.y(), rotaxes.z());
		exprotinverse.multRotate(angles.x() * -M_PI, rotaxes.x(), rotaxes.y(), 0);
		exprotinverse.multScale(1 / scale, 1 / scale, 1 / scale);

		drawObject(explosionmesh, exprot *= exptrans, exprotinverse, explosioncommands[o.getExplosionState()]);

		modeltrans = Matrix3D().setScale(1.0f - turnpercent, 1.0f - turnpercent, 1.0f - turnpercent) *= modeltrans;
		modelinverse.multScale(1 / (1.0f - turnpercent), 1 / (1.0f - turnpercent), 1 / (1.0f - turnpercent));
	}
	if (o.isExplosionSpawning()) {
		float scale = 1.0f - turnpercent;
		scale = 1.0f - scale * scale * scale;
		modeltrans = Matrix3D().setScale(scale, scale, scale) *= modeltrans;
		modelinverse.multScale(1 / (scale), 1 / (scale), 1 / (scale));
	}
	if (o.isPastObjectPicked()) {
		if (o.getPastObject() == SapphireObject::Earth) {
			//random direction
			SapphireRandom dirtrandom { level->getTurn() * 1049 + i * 877 + j * 761 };
			int count = (int) (5 + dirtrandom.next(2));
			for (int i = 0; i < count; ++i) {
				float x = (dirtrandom.nextFloat() - 0.5f);
				float y = (dirtrandom.nextFloat() - 0.5f);

				float dirtscale = 1.35f - turnpercent * 0.7f;

				Vector3F offset = Vector3F { (dirtrandom.nextFloat() - 0.5f) * 2, (dirtrandom.nextFloat() - 0.2f) * 3,
						(dirtrandom.nextFloat() - 0.3f) / 1.5f };
				offset += directionToVector(o.getPastObjectPickDirection());
				offset.normalize();
				offset *= turnpercent;
				offset.y() += -2.0f / 2.0f * turnpercent * turnpercent;
				drawObject(earth_dirtmesh,
						Matrix3D().setScale(dirtscale, dirtscale, dirtscale).multTranslate(offset) *= Matrix3D().setTranslate(x, y, 0) *=
								exptrans,
						Matrix3D(Matrix3D(expinverse).multTranslate(-x, -y, 0)).multTranslate(offset).multScale(1 / dirtscale,
								1 / dirtscale, 1 / dirtscale), dirtCommands);

			}
		} else {
			Matrix3D picktrans = exptrans;
			Matrix3D pickinverse = expinverse;
			multMatrixTranslateToDirection(picktrans, pickinverse, o.getPastObjectPickDirection(), 1 - turnpercent);
			float scale = 1 - turnpercent;
			scale *= scale;
			auto* pastobj = getPast3DObject(o.getPastObject(), picktrans, pickinverse, turnpercent);
			drawObject(pastobj, Matrix3D().setScale(scale, scale, scale) *= picktrans,
					Matrix3D(pickinverse).multScale(1 / (scale), 1 / (scale), 1 / (scale)));
		}
	}

	if (o.isCitrineShattered()) {
		SapphireRandom random { level->getOriginalRandomSeed() + level->getTurn() * 1049 + i * 877 + j * 761 };
		Sapphire3DObject* objects[] { citrine_shattering_1mesh, citrine_shattering_2mesh, citrine_shattering_3mesh, citrine_shattering_4mesh };
		Vector3F origins[] { objects[0]->getOrigin(), objects[1]->getOrigin(), objects[2]->getOrigin(), objects[3]->getOrigin() };

		Matrix3D citrinetrans;
		citrinetrans.setScale(0.5f + (1 - turnpercent) / 2.0f, 0.5f + (1 - turnpercent) / 2.0f, 0.5f + (1 - turnpercent) / 2.0f);
		citrinetrans *= exptrans;
		Matrix3D citrinetransinv = expinverse;
		citrinetransinv.multScale(1 / (0.5f + (1 - turnpercent) / 2.0f), 1 / (0.5f + (1 - turnpercent) / 2.0f),
				1 / (0.5f + (1 - turnpercent) / 2.0f));

		float angles[] { (float) (-M_PI_2 * (random.nextFloat() + 0.5f)), (float) (M_PI * 0.25f * (random.nextFloat() + 0.5f)),
				(float) (M_PI * 1.25f * (random.nextFloat() + 0.5f)), (float) (-M_PI * (random.nextFloat() - 0.5f)) };

		for (int i = 0; i < 4; ++i) {
			auto* obj = objects[i];
			Vector3F& o = origins[i];

			o.y() = (o.y() + 0.5f) * (1 - turnpercent) - 0.5f;
			o.x() = o.x() * (1.0f + turnpercent / 2.0f);

			float rand[] { random.nextFloat(), random.nextFloat(), random.nextFloat() };

			minerSinkCommands.add(obj,
					(Matrix3D().setRotate(angles[i] * turnpercent, rand[0] - 0.5f, rand[1] - 0.5f, 0.3f + 0.2f * rand[2]) *= citrinetrans).multTranslate(
							o),
					Matrix3D(citrinetransinv).multTranslate(-o).multRotate(-angles[i] * turnpercent, rand[0] - 0.5f, rand[1] - 0.5f,
							0.3f + 0.2f * rand[2]), drawCommandCache);
		}
	} else if (o.isCitrineBreaking()) {
		drawBuildingObject(citrine_breakingmesh, exptrans, expinverse, 1.0f - turnpercent);
	}

	switch (o.object) {
		case SapphireObject::Air:
			break;
		case SapphireObject::Emerald: {
			if (o.state == SapphireState::Dispensing) {
				break;
			}
			MULT_ROLLING(M_PI);
			if (o.state == SapphireState::BagOpening) {
				float percent = 1 - turnpercent;
				percent = 1 - percent * percent;
				drawBuildingObject(bagmesh, exptrans, expinverse, 1.0f - percent);
				drawObject(emeraldmesh, Matrix3D().setScale(percent, percent, percent) *= modeltrans,
						Matrix3D(modelinverse).multScale(1 / percent, 1 / percent, 1 / percent));
			} else {
				drawObject(emeraldmesh, modeltrans, modelinverse);
			}
			break;
		}
		case SapphireObject::Ruby: {
			if (o.state == SapphireState::Dispensing) {
				break;
			}
			MULT_ROLLING(M_PI);
			drawObject(rubymesh, modeltrans, modelinverse);
			break;
		}
		case SapphireObject::Sapphire: {
			if (o.state == SapphireState::Dispensing) {
				break;
			}
			MULT_ROLLING(M_PI);
			drawObject(sapphiremesh, modeltrans, modelinverse);
			break;
		}
		case SapphireObject::Citrine: {
			if (o.state == SapphireState::Dispensing) {
				break;
			}
			MULT_ROLLING(M_PI);
			drawObject(citrinemesh, modeltrans, modelinverse);
			break;
		}
		case SapphireObject::Bug: {
			DRAW_PAST_OBJECT_FALL_INTO();
			drawBug(turnpercent, modeltrans, modelinverse);
			break;
		}
		case SapphireObject::KeyBlue: {
			drawKey(turnpercent, key_bluemesh, modeltrans, modelinverse, level->getTurn());
			break;
		}
		case SapphireObject::KeyRed: {
			drawKey(turnpercent, key_redmesh, modeltrans, modelinverse, level->getTurn());
			break;
		}
		case SapphireObject::KeyGreen: {
			drawKey(turnpercent, key_greenmesh, modeltrans, modelinverse, level->getTurn());
			break;
		}
		case SapphireObject::KeyYellow: {
			drawKey(turnpercent, key_yellowmesh, modeltrans, modelinverse, level->getTurn());
			break;
		}
		case SapphireObject::DoorOneTime: {
			if (o.isOneTimeDoorClosed()) {
				drawDoor(turnpercent, door_cover_onetimemesh, door_onetime_leftmesh, door_onetime_rightmesh, modeltrans, modelinverse, o);
			} else if (o.isUsingDoor()) {
				//TODO
				drawDoor(turnpercent, door_cover_onetimemesh, door_onetime_leftmesh, door_onetime_rightmesh, modeltrans, modelinverse, o);
			} else {
				auto&& mesh = o.isOneTimeDoorClosed() ? door_cover_onetime_closedmesh : door_cover_onetimemesh;
				drawDoor(turnpercent, mesh, door_onetime_leftmesh, door_onetime_rightmesh, modeltrans, modelinverse, o);
			}
			break;
		}
		case SapphireObject::DoorBlue: {
			drawDoor(turnpercent, door_cover_bluemesh, door_blue_leftmesh, door_blue_rightmesh, modeltrans, modelinverse, o);
			break;
		}
		case SapphireObject::DoorRed: {
			drawDoor(turnpercent, door_cover_redmesh, door_red_leftmesh, door_red_rightmesh, modeltrans, modelinverse, o);
			break;
		}
		case SapphireObject::DoorGreen: {
			drawDoor(turnpercent, door_cover_greenmesh, door_green_leftmesh, door_green_rightmesh, modeltrans, modelinverse, o);
			break;
		}
		case SapphireObject::DoorYellow: {
			drawDoor(turnpercent, door_cover_yellowmesh, door_yellow_leftmesh, door_yellow_rightmesh, modeltrans, modelinverse, o);
			break;
		}
		case SapphireObject::Player: {
			if (o.isPlayerUsingDoor()) {
				switch (o.direction) {
					case SapphireDirection::Down:
						modeltrans.multTranslate(0.0f, 1 - turnpercent, 0.0f);
						modelinverse = Matrix3D { }.setTranslate(0.0f, -(1 - turnpercent), 0.0f) *= modelinverse;
						break;
					case SapphireDirection::Up:
						modeltrans.multTranslate(0.0f, -1 + turnpercent, 0.0f);
						modelinverse = Matrix3D { }.setTranslate(0.0f, -(-1 + turnpercent), 0.0f) *= modelinverse;
						break;
					case SapphireDirection::Left:
						modeltrans.multTranslate(1 - turnpercent, 0.0f, 0.0f);
						modelinverse = Matrix3D { }.setTranslate(-(1 - turnpercent), 0.0f, 0.0f) *= modelinverse;
						break;
					case SapphireDirection::Right:
						modeltrans.multTranslate(-1 + turnpercent, 0.0f, 0.0f);
						modelinverse = Matrix3D { }.setTranslate(-(-1 + turnpercent), 0.0f, 0.0f) *= modelinverse;
						break;
					default:
						break;
				}
			}
			DRAW_PAST_OBJECT_FALL_INTO();
			if (o.isPlayerRobotKilled() && o.getPastObject() == SapphireObject::Robot) {
				Matrix3D robottrans = exptrans;
				Matrix3D robotinverse;

				switch (o.getPlayerRobotKillDirection()) {
					case SapphireDirection::Down:
						robottrans.multTranslate(0.0f, 1 - turnpercent, 0);
						robotinverse.setTranslate(0.0f, -1 + turnpercent, 0);
						break;
					case SapphireDirection::Up:
						robottrans.multTranslate(0.0f, -1 + turnpercent, 0);
						robotinverse.setTranslate(0.0f, 1 - turnpercent, 0);
						break;
					case SapphireDirection::Left:
						robottrans.multTranslate(1 - turnpercent, 0.0f, 0);
						robotinverse.setTranslate(-1 + turnpercent, 0.0f, 0);
						break;
					case SapphireDirection::Right:
						robottrans.multTranslate(-1 + turnpercent, 0.0f, 0);
						robotinverse.setTranslate(1 - turnpercent, 0.0f, 0);
						break;
					default:
						THROW()<< "Invalid direction for robot to kill" << o.direction;
						break;
					}

				drawRobot(turnpercent, robottrans, robotinverse *= expinverse, level->getTurn(), defaultCommands);
			}
			drawMiner(o.getPlayerId() == 0 ? miner1mesh : miner2mesh, turnpercent, level->getTurn(), modeltrans, modelinverse, o.direction,
					o.getPlayerFacing(), o.isPlayerDigging(), o.isMoving(), o.isPlayerTryPush());
			break;
		}
		case SapphireObject::Robot: {
			DRAW_PAST_OBJECT_FALL_INTO();
			drawRobot(turnpercent, modeltrans, modelinverse, level->getTurn(), defaultCommands);
			break;
		}
		case SapphireObject::Rock: {
			if (o.state == SapphireState::Dispensing) {
				break;
			}
			if (o.isSapphireBreaking()) {
				drawBuildingObject(sapphire_breakmesh, exptrans, expinverse, 1.0f - turnpercent);
			}
			MULT_ROLLING(M_PI * 2 / 3);
			drawObject(rockmesh, modeltrans, modelinverse);
			break;
		}
		case SapphireObject::Bag: {
			if (o.state == SapphireState::Dispensing) {
				break;
			}
			MULT_ROLLING(M_PI * 2);
			drawObject(bagmesh, modeltrans, modelinverse);
			break;
		}
		case SapphireObject::Lorry: {
			DRAW_PAST_OBJECT_FALL_INTO();
			drawObject(lorrymesh, modeltrans, modelinverse);
			break;
		}
		case SapphireObject::Swamp: {
			float side = (i + j) % 4;
			modeltrans = Matrix3D().setRotate(M_PI_2 * side, 0, 1, 0) *= modeltrans;
			modelinverse.multRotate(-M_PI_2 * side, 0, 1, 0);

			if (o.isSwampSpawnUp()) {
				drawBuildingObject(swampmesh, modeltrans, modelinverse, turnpercent);
			} else if (o.isSwampDropHit()) {
				drawDropHit(turnpercent, modeltrans, modelinverse, defaultCommands);
				drawBuildingObject(swampmesh, modeltrans, modelinverse, turnpercent);
			} else {
				if (o.isSwampHighlight()) {
					float percent = sinf(turnpercent * M_PI);
					float scalepercent = 1 + percent * 0.12f;
					modeltrans = Matrix3D().setScale(scalepercent, scalepercent, scalepercent) *= modeltrans;
					modelinverse.multScale(1 / scalepercent, 1 / scalepercent, 1 / scalepercent);
				}
				drawObject(swampmesh, modeltrans, modelinverse);
			}
			break;
		}
		case SapphireObject::Wall: {
			drawObject(wallmesh, modeltrans, modelinverse);
			break;
		}
		case SapphireObject::Glass: {
			drawObject(glassmesh, modeltrans, modelinverse);
			break;
		}
		case SapphireObject::StoneWall: {
			drawSidedObject(o, stonewallSidedMesh, modeltrans, modelinverse, [&](const Level::GameObject& o) {
				return o.object == SapphireObject::StoneWall || (o.object == SapphireObject::RoundStoneWall && o.y == j + 1);
			});
			if (o.isStoneWallWithObject()) {
				const float emrot = M_PI_2 * (i * 3) * (j * 5);
				switch (o.getStoneWallObject()) {
					case SapphireObject::Emerald: {
						drawObject(stonewall2_emerald_partmesh, Matrix3D().setRotate(emrot, 0, 0, 1) *= modeltrans,
								Matrix3D(modelinverse).multRotate(emrot, 0, 0, -1));
						break;
					}
					case SapphireObject::Sapphire: {
						drawObject(stonewall2_sapphire_partmesh, Matrix3D().setRotate(emrot, 0, 0, 1) *= modeltrans,
								Matrix3D(modelinverse).multRotate(emrot, 0, 0, -1));
						break;
					}
					case SapphireObject::Ruby: {
						drawObject(stonewall2_ruby_partmesh, Matrix3D().setRotate(emrot, 0, 0, 1) *= modeltrans,
								Matrix3D(modelinverse).multRotate(emrot, 0, 0, -1));
						break;
					}
					case SapphireObject::Citrine: {
						drawObject(stonewall2_citrine_partmesh, Matrix3D().setRotate(emrot, 0, 0, 1) *= modeltrans,
								Matrix3D(modelinverse).multRotate(emrot, 0, 0, -1));
						break;
					}
					default: {
						THROW()<< "Unknown stone wall object: " << o.getStoneWallObject();
						break;
					}
				}
			}
			break;
		}
		case SapphireObject::InvisibleStoneWall: {
			drawSidedObject(o, invisiblestonewallSidedMesh, modeltrans, modelinverse, [](const Level::GameObject& o) {
				return o.object == SapphireObject::InvisibleStoneWall;
			});
			break;
		}
		case SapphireObject::RoundStoneWall: {
			auto* down = level->getOptional(i, j - 1);
			if (down != nullptr && !down->isAnyExplosion() && (down->object == SapphireObject::StoneWall)) {
				drawObject(roundstonewall_downopenmesh, modeltrans, modelinverse);
			}
			drawObject(roundstonewallmesh, modeltrans, modelinverse);
			break;
		}
		case SapphireObject::TickBomb: {
			if (o.isTickBombSpawning()) {
				float scale = 1.0f - turnpercent;
				scale = 1.0f - scale * scale;
				modeltrans = Matrix3D().setScale(scale, scale, scale).multRotate((1 - scale) * M_PI_4, 0, 0, 1) *= modeltrans;
				modelinverse.multRotate(-(1 - scale) * M_PI_4, 0, 0, 1).multScale(1 / (scale), 1 / (scale), 1 / (scale));
			}
			drawTickBomb(true, turnpercent < 0.5f, modeltrans, modelinverse, defaultCommands);
			break;
		}
		case SapphireObject::TimeBomb: {
			drawTickBomb(false, false, modeltrans, modelinverse, defaultCommands);
			break;
		}
		case SapphireObject::Bomb: {
			if (o.state == SapphireState::Dispensing) {
				break;
			}
			MULT_ROLLING(M_PI);
			drawObject(bombmesh, modeltrans, modelinverse);
			break;
		}
		case SapphireObject::PusherRight: {
			drawObject(pusher_rightmesh, modeltrans, modelinverse);
			break;
		}
		case SapphireObject::PusherLeft: {
			drawObject(pushermesh, modeltrans, modelinverse);
			break;
		}
		case SapphireObject::Wheel: {
			drawObject(wheelmesh, modeltrans, modelinverse);
			break;
		}
		case SapphireObject::Drop: {
			if (o.state == SapphireState::Dispensing) {
				break;
			}
			if (o.state == SapphireState::Spawning) {
				modeltrans = Matrix3D().setScale(0.5f + turnpercent / 2, 0.5f + turnpercent / 2, 0.5f + turnpercent / 2) *= modeltrans;
				modelinverse.multScale(1 / (0.5f + turnpercent / 2), 1 / (0.5f + turnpercent / 2), 1 / (0.5f + turnpercent / 2));

				switch (o.direction) {
					case SapphireDirection::Left: {
						modeltrans = Matrix3D().setRotate((1 - turnpercent) * M_PI_2, 0, 0, 1) *= modeltrans;
						modeltrans.multTranslate((1 - turnpercent), 0.0f, 0.0f);

						modelinverse = Matrix3D().setTranslate(-(1 - turnpercent), 0.0f, 0.0f) * modelinverse;
						modelinverse.multRotate(-(1 - turnpercent) * M_PI_2, 0, 0, 1);
						break;
					}
					case SapphireDirection::Right: {
						modeltrans = Matrix3D().setRotate(-(1 - turnpercent) * M_PI_2, 0, 0, 1) *= modeltrans;
						modeltrans.multTranslate(-1 + turnpercent, 0.0f, 0.0f);

						modelinverse = Matrix3D().setTranslate(-(-1 + turnpercent), 0.0f, 0.0f) * modelinverse;
						modelinverse.multRotate((1 - turnpercent) * M_PI_2, 0, 0, 1);
						break;
					}
					case SapphireDirection::Down: {
						modeltrans.multTranslate(0.0f, 1 - turnpercent, 0.0f);

						modelinverse = Matrix3D { }.setTranslate(0.0f, -(1 - turnpercent), 0.0f) *= modelinverse;
						break;
					}
					default: {
						break;
					}
				}
			} else {
				float amount = sinf(turnpercent * M_PI) / 20.0f;
				if ((i + j) % 2 == 0) {
					amount = -amount;
				}
				modeltrans = Matrix3D().setScale(1 + amount, 1 - amount, 1 + amount) * modeltrans;
				modelinverse.multScale(1 / (1 + amount), 1 / (1 - amount), 1 / (1 + amount));
			}
			drawObject(dropmesh, modeltrans, modelinverse);
			break;
		}
		case SapphireObject::Cushion: {
			if (o.isFallCushion()) {
				float scale = 1 + sinf(turnpercent * M_PI) / 8;
				modeltrans = Matrix3D().setScale(scale, 1 / scale, scale) *= modeltrans;
				modelinverse.multScale(1 / scale, scale, 1 / scale);
			}
			drawObject(cushionmesh, modeltrans, modelinverse);
			break;
		}
		case SapphireObject::Acid: {
			DRAW_PAST_OBJECT_FALL_INTO();
			drawAcid(turnpercent, o, modeltrans, modelinverse);
			break;
		}
		case SapphireObject::TNT: {
			drawObject(tntmesh, modeltrans, modelinverse);
			break;
		}
		case SapphireObject::Exit: {
			if (o.isExitSinkPlayer()) {
				drawMiner(o.getPlayerId() == 0 ? miner1mesh : miner2mesh, turnpercent, level->getTurn(), modeltrans, modelinverse,
						SapphireDirection::Up, o.direction, false, true, false, minerSinkCommands);
			}
			if (o.isExitWalkPlayer()) {
				Matrix3D plrmodel;
				Matrix3D plrmodelinverse = modelinverse;

				float ztrans = sinf(turnpercent * M_PI) * 0.3f;

				switch (o.direction) {
					case SapphireDirection::Down:
						plrmodel.setTranslate(0.0f, 1 - turnpercent, ztrans);
						plrmodelinverse.multTranslate(0.0f, 1 - turnpercent, -ztrans);
						break;
					case SapphireDirection::Up:
						plrmodel.setTranslate(0.0f, -1 + turnpercent, ztrans);
						plrmodelinverse.multTranslate(0.0f, -1 + turnpercent, -ztrans);
						break;
					case SapphireDirection::Left:
						plrmodel.setTranslate(1 - turnpercent, 0.0f, ztrans);
						plrmodelinverse.multTranslate(1 - turnpercent, 0.0f, -ztrans);
						break;
					case SapphireDirection::Right:
						plrmodel.setTranslate(-1 + turnpercent, 0.0f, ztrans);
						plrmodelinverse.multTranslate(-1 + turnpercent, 0.0f, -ztrans);
						break;
					default: {
						THROW()<< "Invalid direction for player to exit " << o.direction;
						break;
					}
				}
				plrmodel *= modeltrans;
				drawMiner(o.getPlayerId() == 0 ? miner1mesh : miner2mesh, turnpercent, level->getTurn(), plrmodel, plrmodelinverse,
						o.direction, o.direction, false, true, false);
			}
			switch (o.getExitState()) {
				case SapphireDynamic::ExitOccupied: {
					drawExit(1.0f - turnpercent, modeltrans, modelinverse);
					break;
				}
				case SapphireDynamic::ExitClosed: {
					drawExit(0.0f, modeltrans, modelinverse);
					break;
				}
				case SapphireDynamic::ExitClosing: {
					drawExit(1.0f, modeltrans, modelinverse);
					break;
				}
				case SapphireDynamic::ExitOpening: {
					drawExit(turnpercent, modeltrans, modelinverse);
					break;
				}
				case SapphireDynamic::ExitOpen: {
					drawExit(1.0f, modeltrans, modelinverse);
					break;
				}
				default: {
					break;
				}
			}
			break;
		}
		case SapphireObject::YamYam: {
			DRAW_PAST_OBJECT_FALL_INTO();
			if (o.state == SapphireState::Still || o.state == SapphireState::Taking) {
				drawYamYam(sinf(turnpercent * M_PI), turnpercent, modeltrans, modelinverse, o.getYamYamOldDirection(), o.direction,
						defaultCommands);
			} else {
				drawYamYam(0.0f, turnpercent, modeltrans, modelinverse, o.getYamYamOldDirection(), o.direction, defaultCommands);
			}
			break;
		}
		case SapphireObject::Elevator: {
			drawElevator(o.state != SapphireState::Still && turnpercent < 0.5f, modeltrans, modelinverse);
			break;
		}
		case SapphireObject::Earth: {
			drawSidedObject(o, earth2SidedMesh, modeltrans, modelinverse, [](const Level::GameObject& o) {
				return o.object == SapphireObject::Earth;
			});
			break;
		}
		case SapphireObject::Safe: {
			drawObject(safemesh, modeltrans, modelinverse);
			break;
		}
		case SapphireObject::Converter: {
			DRAW_PAST_OBJECT_FALL_INTO();
			drawConverter(turnpercent, o, modeltrans, modelinverse);
			break;
		}
		case SapphireObject::Sand: {
//float angle = (i + j) * M_PI_2;
//modeltrans = Matrix3D().setRotate(angle, 0, 1, 0) *= modeltrans;
//modelinverse.multRotate(angle, 0, -1, 0);
//drawObject(sand2mesh, modeltrans, modelinverse);
			drawSand(o, modeltrans, modelinverse);
			break;
		}
		case SapphireObject::SandRock: {
			DRAW_PAST_OBJECT_FALL_INTO() else {
				drawObject(rockmesh, modeltrans, modelinverse);
			}
			drawSand(o, modeltrans, modelinverse);
//drawObject(sand2mesh, modeltrans, modelinverse);
			break;
		}
		case SapphireObject::Dispenser: {
			drawObject(dispensermesh, modeltrans, modelinverse);
			Sapphire3DObject* obj;
			switch (o.getDispenserObject()) {
				case SapphireObject::Emerald: {
					obj = emeraldmesh;
					break;
				}
				case SapphireObject::Sapphire: {
					obj = sapphiremesh;
					break;
				}
				case SapphireObject::Ruby: {
					obj = rubymesh;
					break;
				}
				case SapphireObject::Bomb: {
					obj = bombmesh;
					break;
				}
				case SapphireObject::Bag: {
					obj = bagmesh;
					break;
				}
				case SapphireObject::Drop: {
					obj = dropmesh;
					break;
				}
				case SapphireObject::Citrine: {
					obj = citrinemesh;
					break;
				}
				case SapphireObject::Rock:
				default: {
					obj = rockmesh;
					break;
				}
			}
			if (o.isDispenserSpawn()) {
				//draw falling rock
				drawObject(obj, Matrix3D(modeltrans).multTranslate(0, -turnpercent, 0), modelinverse);
			}
			drawObject(obj, modeltrans, modelinverse, dispenserCommands);
			break;
		}
		default: {
			THROW()<< "Undrawn object" << o.object;
			break;
		}
	}

	if (o.isLaser()) {
		if (HAS_FLAG(o.visual, SapphireVisual::LaserHorizontal)) {
			drawObject(laser_verticalmesh, Matrix3D().setRotate(M_PI_2, 0, 0, 1) *= exptrans,
					Matrix3D(expinverse).multRotate(-M_PI_2, 0, 0, 1), laserCommands);
		}
		if (HAS_FLAG(o.visual, SapphireVisual::LaserVertical)) {
			drawObject(laser_verticalmesh, exptrans, expinverse, laserCommands);
		}
		if (HAS_FLAG(o.visual, SapphireVisual::LaserLeftBottom)) {
			drawObject(laser_leftmesh, exptrans, expinverse, laserCommands);
		}
		if (HAS_FLAG(o.visual, SapphireVisual::LaserLeftTop)) {
			drawObject(laser_leftmesh, Matrix3D().setRotate(M_PI_2, 0, 0, 1) *= exptrans, Matrix3D(expinverse).multRotate(-M_PI_2, 0, 0, 1),
					laserCommands);
		}
		if (HAS_FLAG(o.visual, SapphireVisual::LaserRightTop)) {
			drawObject(laser_leftmesh, Matrix3D().setRotate(M_PI, 0, 0, 1) *= exptrans, Matrix3D(expinverse).multRotate(M_PI, 0, 0, -1),
					laserCommands);
		}
		if (HAS_FLAG(o.visual, SapphireVisual::LaserRightBottom)) {
			drawObject(laser_leftmesh, Matrix3D().setRotate(M_PI, 0, 1, 0) *= exptrans, Matrix3D(expinverse).multRotate(M_PI, 0, -1, 0),
					laserCommands);
		}
	}
}

void LevelDrawer3D::generateBackground() {
	if (needBackground) {
		background.regenerateBackground(this->level->getOriginalRandomSeed());
	}
}

void LevelDrawer3D::levelReloaded() {
	generateBackground();
}

void LevelDrawer3D::disableBackground() {
	this->needBackground = false;
	background.disable();
}

} // namespace userapp

