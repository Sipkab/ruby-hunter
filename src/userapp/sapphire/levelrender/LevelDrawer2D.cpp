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
 * LevelDrawer.cpp
 *
 *  Created on: 2016. apr. 26.
 *      Author: sipka
 */

#include <framework/geometry/Rectangle.h>
#include <framework/geometry/Vector.h>
#include <framework/render/Renderer.h>
#include <gen/types.h>
#include <gen/log.h>
#include <sapphire/FrameAnimation.h>
#include <sapphire/level/Level.h>
#include <sapphire/levelrender/LevelDrawer2D.h>
#include <appmain.h>

#define _USE_MATH_DEFINES
#include <math.h>

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

LevelDrawer2D::LevelDrawer2D(LevelDrawer& parent, const Level* level)
		: level(level) {
}

FrameAnimation::Element LevelDrawer2D::getPlayerElement(MinerAnimations& mineranims, bool pushing, bool digging, bool moving,
		SapphireDirection dir, float turnpercent) {
	if (pushing) {
		switch (dir) {
			case SapphireDirection::Left: {
				return mineranims.pushleftanim->getAtPercent(1 - turnpercent);
			}
			case SapphireDirection::Right: {
				return mineranims.pushleftanim->getAtPercent(1 - turnpercent).flippedY();
			}
			case SapphireDirection::Up: {
				return mineranims.walkupanim->getAtPercent(1 - turnpercent);
			}
			case SapphireDirection::Down: {
				return mineranims.walkdownanim->getAtPercent(1 - turnpercent);
			}
			default: {
				THROW()<< "Invalid push direction";
				return mineranims.anim->getAtIndex(0);
			}
		}
	} else if (digging) {
		switch (dir) {
			case SapphireDirection::Left: {
				return mineranims.digleftanim->getAtPercent(1 - turnpercent);
			}
			case SapphireDirection::Right: {
				return mineranims.digleftanim->getAtPercent(1 - turnpercent).flippedY();
			}
			case SapphireDirection::Up: {
				return mineranims.digupanim->getAtPercent(1 - turnpercent);
			}
			case SapphireDirection::Down: {
				return mineranims.digdownanim->getAtPercent(1 - turnpercent);
			}
			default: {
				THROW()<< "no direction for digging";
				return mineranims.anim->getAtIndex(0);
			}
		}
	} else if (moving) {
		switch (dir) {
			case SapphireDirection::Left: {
				return (mineranims.walkleftanim)->getAtPercent(1 - turnpercent);
			}
			case SapphireDirection::Right: {
				return (mineranims.walkleftanim)->getAtPercent(1 - turnpercent).flippedY();
			}
			case SapphireDirection::Up: {
				return (mineranims.walkupanim)->getAtPercent(1 - turnpercent);
			}
			case SapphireDirection::Down: {
				return (mineranims.walkdownanim)->getAtPercent(1 - turnpercent);
			}
			default: {
				THROW() << "Invalid move direction";
				return mineranims.anim->getAtIndex(0);
			}
		}
	} else {
		return mineranims.anim->getAtIndex(0);
	}
}

FrameAnimation::Element LevelDrawer2D::getPlayerElement(const Level::GameObject& o, float turnpercent) {
	return getPlayerElement(o.getPlayerId() == 0 ? man1Animations : man2Animations, o.isPlayerTryPush(), o.isPlayerDigging(), o.isMoving(),
			o.direction, turnpercent);
}

const FrameAnimation::Element& LevelDrawer2D::getPastObjectElement(const Level::GameObject& o, float turnpercent) {
	switch (o.getPastObject()) {
		case SapphireObject::Bag: {
			return baganim->getAtIndex(0);
		}
		case SapphireObject::Bomb: {
			return bombanim->getAtPercent(turnpercent);
		}
		case SapphireObject::Emerald: {
			return emeraldanim->getAtIndex(0);
		}
		case SapphireObject::Ruby: {
			return rubyanim->getAtIndex(0);
		}
		case SapphireObject::Sapphire: {
			return sapphireanim->getAtIndex(0);
		}
		case SapphireObject::Citrine: {
			return citrineanim->getAtIndex(0);
		}
		case SapphireObject::Rock: {
			return rockanim->getAtIndex(0);
		}
		case SapphireObject::Drop: {
			return dropanim->getAtIndex(0);
		}
		case SapphireObject::Robot: {
			return robotanim->getAtPercent((level->getTurn() % 2) == 0 ? turnpercent : 1.0f - turnpercent);
			break;
		}
		case SapphireObject::KeyRed: {
			return redkeyanim->getAtPercent(((level->getTurn() % 4) + turnpercent) / 4.0f);
		}
		case SapphireObject::KeyGreen: {
			return greenkeyanim->getAtPercent(((level->getTurn() % 4) + turnpercent) / 4.0f);
		}
		case SapphireObject::KeyBlue: {
			return bluekeyanim->getAtPercent(((level->getTurn() % 4) + turnpercent) / 4.0f);
		}
		case SapphireObject::KeyYellow: {
			return yellowkeyanim->getAtPercent(((level->getTurn() % 4) + turnpercent) / 4.0f);
		}
		default: {
			THROW()<< "Unknwon past object " << o.getPastObject();
			break;
		}
	}
	THROW()<< "Invalid past object" << o.getPastObject();
	return man1Animations.anim->getAtIndex(0);
}

#define TILE_RECT Rectangle { -0.5, -0.5, 0.5, 0.5 }

#define DRAW_PAST_OBJECT_FALL_INTO() \
	if (o.isPastObjectFallInto()) { \
		auto& elem = getPastObjectElement(o, turnpercent); \
		drawSapphireTexturePrepared(Matrix2D { }.setTranslate(0, turnpercent - 1) *= expmvp, elem, alpha, TILE_RECT, elem.getPosition() ); \
	}

template<>
void LevelDrawer2D::drawObject<SapphireObject::Player>(const Level::GameObject& o, const Matrix2D& mvp, const Matrix2D& expmvp, float alpha,
		float turnpercent) {

	//TODO handle 2player here
	auto&& elem = getPlayerElement(o, turnpercent);
	drawSapphireTexturePrepared(mvp, elem.getTexture(), alpha, TILE_RECT, elem.getPosition());
	if (o.isPlayerRobotKilled() && o.getPastObject() == SapphireObject::Robot) {
		Matrix2D robotmvp;

		switch (o.getPlayerRobotKillDirection()) {
			case SapphireDirection::Down:
				robotmvp.setTranslate(0.0f, -1 + turnpercent);
				break;
			case SapphireDirection::Up:
				robotmvp.setTranslate(0.0f, 1 - turnpercent);
				break;
			case SapphireDirection::Left:
				robotmvp.setTranslate(1 - turnpercent, 0.0f);
				break;
			case SapphireDirection::Right:
				robotmvp.setTranslate(-1 + turnpercent, 0.0f);
				break;
			default: {
				THROW()<< "Invalid direction for robot to kill" << o.direction;
				break;
			}
		}

		auto&& robotelem = robotanim->getAtPercent((level->getTurn() % 2) == 0 ? turnpercent : 1.0f - turnpercent);
		drawSapphireTexturePrepared(robotmvp *= expmvp, robotelem, alpha, TILE_RECT, robotelem.getPosition());
	}
	DRAW_PAST_OBJECT_FALL_INTO();
}

void LevelDrawer2D::drawDoor(const FrameAnimation& anim, const Level::GameObject& o, const Matrix2D& mvp, const Matrix2D& omvp, float alpha,
		float turnpercent) {
	auto& elem = o.isUsingDoor() ? anim.getAtPercent(1.0f - turnpercent) : anim.getAtIndex(0);
	if (o.isUsingDoor()) {
		int i = (int) o.x;
		int j = (int) o.y;
		if (i - 1 >= 0 && level->get(i - 1, j).object == SapphireObject::Player
				&& level->get(i - 1, j).direction == SapphireDirection::Left) {
			Matrix2D plrexpmvp;
			plrexpmvp.setTranslate(0.5f + i - 1, -0.5f + (level->getHeight() - j)) *= omvp;
			drawObject<SapphireObject::Player>(level->get(i - 1, j), Matrix2D { }.setTranslate(turnpercent * -2.0f + 2.0f, 0) *= plrexpmvp,
					plrexpmvp, alpha, turnpercent);
		}
		if (j - 1 >= 0 && level->get(i, j - 1).object == SapphireObject::Player
				&& level->get(i, j - 1).direction == SapphireDirection::Down) {
			Matrix2D plrexpmvp;
			plrexpmvp.setTranslate(0.5f + i, -0.5f + (level->getHeight() - (j - 1))) *= omvp;
			drawObject<SapphireObject::Player>(level->get(i, j - 1), Matrix2D { }.setTranslate(0, turnpercent * 2.0f - 2.0f) *= plrexpmvp,
					plrexpmvp, alpha, turnpercent);
		}
		if (i + 1 < level->getWidth() && level->get(i + 1, j).object == SapphireObject::Player
				&& level->get(i + 1, j).direction == SapphireDirection::Right) {
			Matrix2D plrexpmvp;
			plrexpmvp.setTranslate(0.5f + i + 1, -0.5f + (level->getHeight() - j)) *= omvp;
			drawObject<SapphireObject::Player>(level->get(i + 1, j), Matrix2D { }.setTranslate(turnpercent * 2.0f - 2.0f, 0) *= plrexpmvp,
					plrexpmvp, alpha, turnpercent);
		}
		if (j + 1 < level->getHeight() && level->get(i, j + 1).object == SapphireObject::Player
				&& level->get(i, j + 1).direction == SapphireDirection::Up) {
			Matrix2D plrexpmvp;
			plrexpmvp.setTranslate(0.5f + i, -0.5f + (level->getHeight() - (j + 1))) *= omvp;
			drawObject<SapphireObject::Player>(level->get(i, j + 1), Matrix2D { }.setTranslate(0, turnpercent * -2.0f + 2.0f) *= plrexpmvp,
					plrexpmvp, alpha, turnpercent);
		}
	}
	drawSapphireTexturePrepared(mvp, elem, alpha, TILE_RECT, elem.getPosition());
}

void LevelDrawer2D::drawEarth(const Level::GameObject& o, const Matrix2D& mvp, float alpha) {
	unsigned int index = 0;
	int i = (int) o.x;
	int j = (int) o.y;

	if (i - 1 < 0
			|| (level->get(i - 1, j).object != SapphireObject::Earth && level->get(i - 1, j).getPastObject() != SapphireObject::Earth)) {
		index |= 0x1;
	}
	if (j - 1 < 0
			|| (level->get(i, j - 1).object != SapphireObject::Earth && level->get(i, j - 1).getPastObject() != SapphireObject::Earth)) {
		index |= 0x8;
	}
	if (i + 1 >= level->getWidth()
			|| (level->get(i + 1, j).object != SapphireObject::Earth && level->get(i + 1, j).getPastObject() != SapphireObject::Earth)) {
		index |= 0x4;
	}
	if (j + 1 >= level->getHeight()
			|| (level->get(i, j + 1).object != SapphireObject::Earth && level->get(i, j + 1).getPastObject() != SapphireObject::Earth)) {
		index |= 0x2;
	}

	auto& elem = earthanim->getAtIndex(index);
	drawSapphireTexturePrepared(mvp, elem, alpha, TILE_RECT, elem.getPosition());
}

void LevelDrawer2D::draw(LevelDrawer& parent, float turnpercent, float alpha, const Size2UI& begin, const Size2UI& end, const Vector2F& mid,
		const Size2F& objectSize) {
	prepareSapphireTextureDraw();

	renderer->setDepthTest(false);
	renderer->initDraw();

	Size2F fieldsize = parent.getSize().pixelSize / objectSize;

	Matrix2D omvp;
	omvp.setTranslate(-mid.x(), -(level->getHeight() - mid.y())).multScreenDimension(fieldsize / -2.0f, fieldsize / 2.0f);

	unsigned int dispenserspeed = level->getDispenserRechargeSpeed();
	const float dispenseralpha = alpha
			* (parent.isDispenserFullOpacity() ?
					1.0f : (dispenserspeed == 0 ? 1.0f : ((level->getDispenserValue() % dispenserspeed) + turnpercent) / dispenserspeed));

	for (int i = begin.x(); i < end.x(); ++i) {
		for (int j = begin.y(); j < end.y(); ++j) {
			const Level::GameObject& o = level->get(i, j);
			Matrix2D mvp;
			mvp.setIdentity();
			Matrix2D expmvp;
			expmvp.setTranslate(0.5f + i, -0.5f + (level->getHeight() - j)) *= omvp;

			if (o.object == SapphireObject::Bug) {
				if (o.state == SapphireState::Still && !o.isAnyExplosion()) {
					mvp.multRotate((1 - turnpercent) * (float) (o.isDefaultTurned() ? M_PI_2 : -M_PI_2));
				}
				mvp.multRotate(directionToRadian(o.direction));
			} else if (o.object == SapphireObject::Lorry) {
				if (o.state == SapphireState::Still && !o.isAnyExplosion()) {
					mvp.multRotate((1 - turnpercent) * (float) (o.isDefaultTurned() ? -M_PI_2 : M_PI_2));
				}
				mvp.multRotate(directionToRadian(o.direction));
			} else if (o.object == SapphireObject::Wheel && o.isWheelActive()) {
				mvp.multRotate(turnpercent * (float) M_PI_2);
			}
			//TODO should always multiple the matrix?
			if (o.isMoving()) {
				switch (o.direction) {
					case SapphireDirection::Down:
						mvp.multTranslate(0.0f, -1 + turnpercent);
						break;
					case SapphireDirection::Up:
						mvp.multTranslate(0.0f, 1 - turnpercent);
						break;
					case SapphireDirection::Left:
						mvp.multTranslate(1 - turnpercent, 0.0f);
						break;
					case SapphireDirection::Right:
						mvp.multTranslate(-1 + turnpercent, 0.0f);
						break;
					default:
						break;
				}
			}

			mvp *= expmvp;

			if (o.isPastObjectPicked()) {
				if (o.getPastObject() == SapphireObject::Earth) {
					drawEarth(o, expmvp, alpha);

					auto dir = o.getPastObjectPickDirection();
					auto&& elem = (
							dir == SapphireDirection::Left ?
									earthdigleftanim->getAtPercent(1.0f - turnpercent) :
									(dir == SapphireDirection::Up ?
											earthdigupanim->getAtPercent(1.0f - turnpercent) :
											(dir == SapphireDirection::Right ?
													earthdigleftanim->getAtPercent(1.0f - turnpercent).flippedY() :
													(earthdigupanim->getAtPercent(1.0f - turnpercent).flippedX()))));
					drawSapphireTexturePrepared(expmvp, elem, alpha, TILE_RECT, elem.getPosition());
				} else {
					auto& elem = getPastObjectElement(o, turnpercent);
					drawSapphireTexturePrepared(Matrix2D { }.setScale(1.0f - turnpercent, 1.0f - turnpercent) *= expmvp, elem, alpha,
					TILE_RECT, elem.getPosition());
				}
			}
			if (o.isCitrineBreaking()) {
				auto& elem = citrinebreakanim->getAtPercent(1.0f - turnpercent);
				drawSapphireTexturePrepared(expmvp, elem, alpha, TILE_RECT, elem.getPosition());
			} else if (o.isCitrineShattered()) {
				auto& elem = citrineshatteranim->getAtPercent(1.0f - turnpercent);
				drawSapphireTexturePrepared(expmvp, elem, alpha * (1.0f - turnpercent), TILE_RECT, elem.getPosition());
			}

			switch (o.object) {
				case SapphireObject::Air:
					break;
				case SapphireObject::Player: {
					if (!o.isPlayerUsingDoor()) {
						drawObject<SapphireObject::Player>(o, mvp, expmvp, alpha, turnpercent);
					}
					break;
				}
				case SapphireObject::Glass: {
					auto& elem = glassanim->getAtIndex(0);
					drawSapphireTexturePrepared(mvp, elem, alpha, TILE_RECT, elem.getPosition());
					break;
				}
				case SapphireObject::Bag: {
					if (o.state == SapphireState::Dispensing) {
						break;
					}

					auto& elem =
							o.isRolling() ?
									baganim->getAtPercent(o.direction == SapphireDirection::Left ? turnpercent : 1 - turnpercent) :
									baganim->getAtIndex(0);
					drawSapphireTexturePrepared(mvp, elem, alpha, TILE_RECT, elem.getPosition());
					break;
				}
				case SapphireObject::Converter: {
					DRAW_PAST_OBJECT_FALL_INTO();
					auto& elem = converteranim->getAtPercent(turnpercent);
					drawSapphireTexturePrepared(mvp, elem, alpha, TILE_RECT, elem.getPosition());
					break;
				}
				case SapphireObject::Emerald: {
					if (o.state == SapphireState::Dispensing) {
						break;
					}

					auto& elem =
							o.state == SapphireState::BagOpening ?
									bagopenanim->getAtPercent(1.0f - turnpercent) :
									(o.isRolling() ?
											emeraldanim->getAtPercent(
													o.direction == SapphireDirection::Left ? turnpercent : 1 - turnpercent) :
											emeraldanim->getAtIndex(0));
					drawSapphireTexturePrepared(mvp, elem, alpha, TILE_RECT, elem.getPosition());
					break;
				}
				case SapphireObject::Sapphire: {
					if (o.state == SapphireState::Dispensing) {
						break;
					}

					auto& elem =
							o.isRolling() ?
									sapphireanim->getAtPercent(o.direction == SapphireDirection::Left ? turnpercent : 1 - turnpercent) :
									sapphireanim->getAtIndex(0);
					drawSapphireTexturePrepared(mvp, elem, alpha, TILE_RECT, elem.getPosition());
					break;
				}
				case SapphireObject::Ruby: {
					if (o.state == SapphireState::Dispensing) {
						break;
					}

					auto& elem =
							o.isRolling() ?
									rubyanim->getAtPercent(o.direction == SapphireDirection::Left ? turnpercent : 1 - turnpercent) :
									rubyanim->getAtIndex(0);
					drawSapphireTexturePrepared(mvp, elem, alpha, TILE_RECT, elem.getPosition());
					break;
				}
				case SapphireObject::Citrine: {
					if (o.state == SapphireState::Dispensing) {
						break;
					}

					auto& elem =
							o.isRolling() ?
									citrineanim->getAtPercent(o.direction == SapphireDirection::Left ? turnpercent : 1 - turnpercent) :
									citrineanim->getAtIndex(0);
					drawSapphireTexturePrepared(mvp, elem, alpha, TILE_RECT, elem.getPosition());
					break;
				}
				case SapphireObject::Rock: {
					if (o.state == SapphireState::Dispensing) {
						break;
					}
					if (o.isSapphireBreaking()) {
						auto& elem = sapphirebreakanim->getAtPercent(1.0f - turnpercent);
						drawSapphireTexturePrepared(expmvp, elem, alpha, TILE_RECT, elem.getPosition());
					}
					auto& elem =
							o.isRolling() ?
									rockanim->getAtPercent(o.direction == SapphireDirection::Left ? turnpercent : 1 - turnpercent) :
									rockanim->getAtIndex(0);
					drawSapphireTexturePrepared(mvp, elem, alpha, TILE_RECT, elem.getPosition());
					break;
				}
				case SapphireObject::Drop: {
					if (o.state == SapphireState::Dispensing) {
						break;
					}
					auto& elem = (
							o.state == SapphireState::Spawning ?
									(o.direction == SapphireDirection::Left ?
											droptoleftanim : (o.direction == SapphireDirection::Right ? droptorightanim : droptodownanim)) :
									dropanim)->getAtPercent(1.0f - turnpercent);
					drawSapphireTexturePrepared(mvp, elem, alpha, TILE_RECT, elem.getPosition());
					break;
				}
				case SapphireObject::Swamp: {
					auto& elem =
							o.isSwampHighlight() ?
									swampanim->getAtPercent(turnpercent) :
									(o.isSwampSpawnUp() ?
											droptoupanim->getAtPercent(1.0f - turnpercent) :
											(o.isSwampDropHit() ? drophitanim->getAtPercent(1.0f - turnpercent) : swampanim->getAtIndex(0)));
					drawSapphireTexturePrepared(mvp, elem, alpha, TILE_RECT, elem.getPosition());
					break;
				}
				case SapphireObject::Acid: {
					DRAW_PAST_OBJECT_FALL_INTO();

					auto& elem = acidanim->getAtPercent(((level->getTurn() % 4) + turnpercent) / 4.0f);
					drawSapphireTexturePrepared(mvp, elem, alpha, TILE_RECT, elem.getPosition());
					bool leftwall = i - 1 < 0 || level->get(i - 1, j).object != SapphireObject::Acid;
					bool rightwall = i + 1 >= level->getWidth() || level->get(i + 1, j).object != SapphireObject::Acid;
					if (leftwall) {
						if (rightwall) {
							auto& wallelem = acidbothanim->getAtIndex(0);
							drawSapphireTexturePrepared(mvp, wallelem, alpha, TILE_RECT, wallelem.getPosition());
						} else {
							auto& wallelem = acidleftanim->getAtIndex(0);
							drawSapphireTexturePrepared(mvp, wallelem, alpha, TILE_RECT, wallelem.getPosition());
						}
					} else if (rightwall) {
						auto& wallelem = acidrightanim->getAtIndex(0);
						drawSapphireTexturePrepared(mvp, wallelem, alpha, TILE_RECT, wallelem.getPosition());
					}
					break;
				}
				case SapphireObject::Bomb: {
					if (o.state == SapphireState::Dispensing) {
						break;
					}

					auto& elem =
							o.isRolling() || o.isMoving() ?
									bombanim->getAtPercent(o.direction == SapphireDirection::Left ? turnpercent : 1 - turnpercent) :
									bombanim->getAtIndex(0);
					drawSapphireTexturePrepared(mvp, elem, alpha, TILE_RECT, elem.getPosition());
					break;
				}
				case SapphireObject::Dispenser: {
					auto& elem = dispenseranim->getAtIndex(0);
					drawSapphireTexturePrepared(mvp, elem, alpha, TILE_RECT, elem.getPosition());

					const FrameAnimation::Element* rockelem;
					//auto& rockelem = rockanim->getAtIndex(0);
					switch (o.getDispenserObject()) {
						case SapphireObject::Emerald: {
							rockelem = &emeraldanim->getAtIndex(0);
							break;
						}
						case SapphireObject::Sapphire: {
							rockelem = &sapphireanim->getAtIndex(0);
							break;
						}
						case SapphireObject::Ruby: {
							rockelem = &rubyanim->getAtIndex(0);
							break;
						}
						case SapphireObject::Citrine: {
							rockelem = &citrineanim->getAtIndex(0);
							break;
						}
						case SapphireObject::Bomb: {
							rockelem = &bombanim->getAtIndex(0);
							break;
						}
						case SapphireObject::Bag: {
							rockelem = &baganim->getAtIndex(0);
							break;
						}
						case SapphireObject::Drop: {
							rockelem = &dropanim->getAtIndex(0);
							break;
						}
						case SapphireObject::Rock:
						default: {
							rockelem = &rockanim->getAtIndex(0);
							break;
						}
					}

					drawSapphireTexturePrepared(mvp, *rockelem, Color { 0, 0, 0, alpha }, TILE_RECT, rockelem->getPosition());

					drawSapphireTexturePrepared(mvp, *rockelem, dispenseralpha, TILE_RECT, rockelem->getPosition());

					if (o.isDispenserSpawn()) {
						drawSapphireTexturePrepared(Matrix2D { }.setTranslate(0, turnpercent) *= expmvp, *rockelem, alpha, TILE_RECT,
								rockelem->getPosition());
					}

					break;
				}
				case SapphireObject::TNT: {
					auto& elem = tntanim->getAtIndex(0);
					drawSapphireTexturePrepared(mvp, elem, alpha, TILE_RECT, elem.getPosition());
					break;
				}
				case SapphireObject::Exit: {
					if (o.isExitSinkPlayer()) {
						//TODO draw player based on playerid
						auto&& plrelem = getPlayerElement(o.getPlayerId() == 0 ? man1Animations : man2Animations, false, false, true,
								SapphireDirection::Up, turnpercent);
						drawSapphireTexturePrepared(mvp, plrelem, alpha * (1.0f - turnpercent), TILE_RECT, plrelem.getPosition());
					}
					switch (o.getExitState()) {
						case SapphireDynamic::ExitOccupied: {

							auto& elem = exitanim->getAtPercent(turnpercent);
							drawSapphireTexturePrepared(mvp, elem, alpha, TILE_RECT, elem.getPosition());
							break;
						}
						case SapphireDynamic::ExitClosed: {
							auto& elem = exitanim->getAtIndex(exitanim->getChildCount() - 1);
							drawSapphireTexturePrepared(mvp, elem, alpha, TILE_RECT, elem.getPosition());
							break;
						}
						case SapphireDynamic::ExitClosing: {
							auto& elem = exitanim->getAtIndex(0);
							drawSapphireTexturePrepared(mvp, elem, alpha, TILE_RECT, elem.getPosition());
							break;
						}
						case SapphireDynamic::ExitOpening: {
							auto& elem = exitanim->getAtPercent(1.0f - turnpercent);
							drawSapphireTexturePrepared(mvp, elem, alpha, TILE_RECT, elem.getPosition());
							break;
						}
						case SapphireDynamic::ExitOpen: {
							auto& elem = exitanim->getAtIndex(0);
							drawSapphireTexturePrepared(mvp, elem, alpha, TILE_RECT, elem.getPosition());
							break;
						}
						default: {
							break;
						}
					}
					if (o.isExitWalkPlayer()) {
						Matrix2D plrmvp;

						switch (o.direction) {
							case SapphireDirection::Down:
								plrmvp.setTranslate(0.0f, -1 + turnpercent);
								break;
							case SapphireDirection::Up:
								plrmvp.setTranslate(0.0f, 1 - turnpercent);
								break;
							case SapphireDirection::Left:
								plrmvp.setTranslate(1 - turnpercent, 0.0f);
								break;
							case SapphireDirection::Right:
								plrmvp.setTranslate(-1 + turnpercent, 0.0f);
								break;
							default: {
								THROW()<< "Invalid direction for player to exit " << o.direction;
								break;
							}
						}

						auto&& plrelem = getPlayerElement(o.getPlayerId() == 0 ? man1Animations : man2Animations, false, false, true,
								o.direction, turnpercent);
						drawSapphireTexturePrepared(plrmvp *= expmvp, plrelem, alpha, TILE_RECT, plrelem.getPosition());
					}
					break;
				}
				case SapphireObject::RoundStoneWall: {
					auto& elem = roundstonewallanim->getAtIndex(0);
					drawSapphireTexturePrepared(mvp, elem, alpha, TILE_RECT, elem.getPosition());
					break;
				}
				case SapphireObject::Earth: {
					drawEarth(o, expmvp, alpha);
					break;
				}
				case SapphireObject::StoneWall: {
					auto& elem = stonewallanim->getAtIndex(0);
					drawSapphireTexturePrepared(mvp, elem, alpha, TILE_RECT, elem.getPosition());
					if (o.isStoneWallWithObject()) {
						switch (o.getStoneWallObject()) {
							case SapphireObject::Emerald: {
								auto& elem = stonewallgemmask->getAtIndex(0);
								drawSapphireTexturePrepared(mvp, elem, Color { 0, 1, 0, alpha }, TILE_RECT, elem.getPosition());
								break;
							}
							case SapphireObject::Ruby: {
								auto& elem = stonewallgemmask->getAtIndex(0);
								drawSapphireTexturePrepared(mvp, elem, Color { 1, 0, 0, alpha }, TILE_RECT, elem.getPosition());
								break;
							}
							case SapphireObject::Sapphire: {
								auto& elem = stonewallgemmask->getAtIndex(0);
								drawSapphireTexturePrepared(mvp, elem, Color { 0, 0.375, 1, alpha }, TILE_RECT, elem.getPosition());
								break;
							}
							case SapphireObject::Citrine: {
								auto& elem = stonewallgemmask->getAtIndex(0);
								drawSapphireTexturePrepared(mvp, elem, Color { 1, 1, 0, alpha }, TILE_RECT, elem.getPosition());
								break;
							}
							default: {
								break;
							}
						}
					}
					break;
				}
				case SapphireObject::Wall: {
					auto& elem = wallanim->getAtIndex(0);
					drawSapphireTexturePrepared(mvp, elem, alpha, TILE_RECT, elem.getPosition());
					break;
				}
				case SapphireObject::Safe: {
					auto& elem = safeanim->getAtIndex(0);
					drawSapphireTexturePrepared(mvp, elem, alpha, TILE_RECT, elem.getPosition());
					break;
				}
				case SapphireObject::TimeBomb: {
					auto& elem = timebombanim->getAtIndex(0);
					drawSapphireTexturePrepared(mvp, elem, alpha, TILE_RECT, elem.getPosition());
					break;
				}
				case SapphireObject::TickBomb: {
					auto& elem = tickingbombanim->getAtPercent(turnpercent);
					drawSapphireTexturePrepared(mvp, elem, alpha, TILE_RECT, elem.getPosition());
					break;
				}
				case SapphireObject::Sand: {
					auto& elem = sandanim->getAtIndex(0);
					drawSapphireTexturePrepared(mvp, elem, alpha, TILE_RECT, elem.getPosition());
					break;
				}
				case SapphireObject::SandRock: {
					DRAW_PAST_OBJECT_FALL_INTO() else {
						auto& rockelem = rockanim->getAtIndex(0);
						drawSapphireTexturePrepared(mvp, rockelem, alpha, TILE_RECT, rockelem.getPosition());
					}
					auto& sandelem = sandanim->getAtIndex(0);
					drawSapphireTexturePrepared(mvp, sandelem, alpha, TILE_RECT, sandelem.getPosition());
					break;
				}
				case SapphireObject::Cushion: {
					auto& elem = o.isFallCushion() ? cushionanim->getAtPercent(1 - turnpercent) : cushionanim->getAtIndex(0);
					drawSapphireTexturePrepared(mvp, elem, alpha, TILE_RECT, elem.getPosition());
					break;
				}
				case SapphireObject::DoorOneTime: {
					if (o.isUsingDoor()) {
						drawDoor(onetimedooranim, o, mvp, omvp, alpha, turnpercent);
					} else if (o.isOneTimeDoorClosed()) {
						auto&& elem = onetimedooranim->getFirst();
						drawSapphireTexturePrepared(mvp, elem, alpha, TILE_RECT, elem.getPosition());
					} else {
						auto&& elem = onetimedooranim->getLast();
						drawSapphireTexturePrepared(mvp, elem, alpha, TILE_RECT, elem.getPosition());
					}
					break;
				}
				case SapphireObject::DoorRed: {
					drawDoor(reddooranim, o, mvp, omvp, alpha, turnpercent);
					break;
				}
				case SapphireObject::DoorGreen: {
					drawDoor(greendooranim, o, mvp, omvp, alpha, turnpercent);
					break;
				}
				case SapphireObject::DoorBlue: {
					drawDoor(bluedooranim, o, mvp, omvp, alpha, turnpercent);
					break;
				}
				case SapphireObject::DoorYellow: {
					drawDoor(yellowdooranim, o, mvp, omvp, alpha, turnpercent);
					break;
				}
				case SapphireObject::KeyRed: {
					auto& elem = redkeyanim->getAtPercent(((level->getTurn() % 4) + turnpercent) / 4.0f);
					drawSapphireTexturePrepared(mvp, elem, alpha, TILE_RECT, elem.getPosition());
					break;
				}
				case SapphireObject::KeyGreen: {
					auto& elem = greenkeyanim->getAtPercent(((level->getTurn() % 4) + turnpercent) / 4.0f);
					drawSapphireTexturePrepared(mvp, elem, alpha, TILE_RECT, elem.getPosition());
					break;
				}
				case SapphireObject::KeyBlue: {
					auto& elem = bluekeyanim->getAtPercent(((level->getTurn() % 4) + turnpercent) / 4.0f);
					drawSapphireTexturePrepared(mvp, elem, alpha, TILE_RECT, elem.getPosition());
					break;
				}
				case SapphireObject::KeyYellow: {
					auto& elem = yellowkeyanim->getAtPercent(((level->getTurn() % 4) + turnpercent) / 4.0f);
					drawSapphireTexturePrepared(mvp, elem, alpha, TILE_RECT, elem.getPosition());
					break;
				}
				case SapphireObject::YamYam: {
					auto& elem =
							o.state == SapphireState::Still ?
									yamyamanim->getAtPercent(1.0f - turnpercent) : yamyamdirsanim->getAtIndex((unsigned int) o.direction);
					drawSapphireTexturePrepared(mvp, elem, alpha, TILE_RECT, elem.getPosition());
					DRAW_PAST_OBJECT_FALL_INTO();
					break;
				}
				case SapphireObject::Bug: {
					auto& elem = buganim->getAtPercent(((level->getTurn() % 2) + (1.0f - turnpercent)) / 2.0f);
					drawSapphireTexturePrepared(mvp, elem, alpha, TILE_RECT, elem.getPosition());
					DRAW_PAST_OBJECT_FALL_INTO();
					break;
				}
				case SapphireObject::Lorry: {
					auto& elem = lorryanim->getAtPercent(((level->getTurn() % 2) + (1.0f - turnpercent)) / 2.0f);
					drawSapphireTexturePrepared(mvp, elem, alpha, TILE_RECT, elem.getPosition());
					DRAW_PAST_OBJECT_FALL_INTO();
					break;
				}
				case SapphireObject::Wheel: {
					auto& elem = wheelanim->getAtIndex(0);
					drawSapphireTexturePrepared(mvp, elem, alpha, TILE_RECT, elem.getPosition());
					break;
				}
				case SapphireObject::Robot: {
					auto& elem = robotanim->getAtPercent((level->getTurn() % 2) == 0 ? turnpercent : 1.0f - turnpercent);
					drawSapphireTexturePrepared(mvp, elem, alpha, TILE_RECT, elem.getPosition());
					DRAW_PAST_OBJECT_FALL_INTO();
					break;
				}
				case SapphireObject::PusherLeft: {
					auto& elem = pusherleftanim->getAtIndex(0);
					drawSapphireTexturePrepared(mvp, elem, alpha, TILE_RECT, elem.getPosition());
					break;
				}
				case SapphireObject::PusherRight: {
					auto& elem = pusherrightanim->getAtIndex(0);
					drawSapphireTexturePrepared(mvp, elem, alpha, TILE_RECT, elem.getPosition());
					break;
				}
				case SapphireObject::Elevator: {
					auto& elem = o.state == SapphireState::Still ? elevatoranim->getAtIndex(0) : elevatoranim->getAtPercent(turnpercent);
					drawSapphireTexturePrepared(mvp, elem, alpha, TILE_RECT, elem.getPosition());
					break;
				}
					//case SapphireObject::InvisibleWall: //TODO create object as invisible unblowable wall
				case SapphireObject::InvisibleStoneWall: {
					auto& elem = darkwallanim->getAtIndex(0);
					drawSapphireTexturePrepared(mvp, elem, alpha, TILE_RECT, elem.getPosition());
					break;
				}
				default: {
					THROW()<<"Unknown object to draw: " << o.object;
					break;
				}
			}

			if (o.isAnyExplosion() || o.isPropagateExplosion()) {
				auto& elem = explosionanim->getAtPercent(o.getExplosionState() / 4.0f + turnpercent / 4.0f);
				drawSapphireTexturePrepared(expmvp, elem, alpha, TILE_RECT, elem.getPosition());
			}
			if (o.isLaser()) {
				auto& vert = laservertanim->getAtPercent(turnpercent);
				auto& botright = laserbotrightanim->getAtPercent(turnpercent);
				if (HAS_FLAG(o.visual, SapphireVisual::LaserHorizontal)) {
					drawSapphireTexturePrepared(Matrix2D { }.setRotate((float) M_PI_2) *= expmvp, vert, alpha, TILE_RECT,
							vert.getPosition());
				}
				if (HAS_FLAG(o.visual, SapphireVisual::LaserVertical)) {
					drawSapphireTexturePrepared(expmvp, vert, alpha, TILE_RECT, vert.getPosition());
				}
				if (HAS_FLAG(o.visual, SapphireVisual::LaserLeftBottom)) {
					drawSapphireTexturePrepared(Matrix2D { }.setRotate((float) M_PI_2 * 3.0f) *= expmvp, botright, alpha, TILE_RECT,
							botright.getPosition());
				}
				if (HAS_FLAG(o.visual, SapphireVisual::LaserLeftTop)) {
					drawSapphireTexturePrepared(Matrix2D { }.setRotate((float) M_PI) *= expmvp, botright, alpha, TILE_RECT,
							botright.getPosition());
				}
				if (HAS_FLAG(o.visual, SapphireVisual::LaserRightTop)) {
					drawSapphireTexturePrepared(Matrix2D { }.setRotate((float) M_PI_2) *= expmvp, botright, alpha, TILE_RECT,
							botright.getPosition());
				}
				if (HAS_FLAG(o.visual, SapphireVisual::LaserRightBottom)) {
					drawSapphireTexturePrepared(expmvp, botright, alpha, TILE_RECT, botright.getPosition());
				}
			}
		}
	}

}

}  // namespace userapp

