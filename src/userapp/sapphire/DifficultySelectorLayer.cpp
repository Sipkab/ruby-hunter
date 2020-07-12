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
 * DifficultySelectorLayer.cpp
 *
 *  Created on: 2016. apr. 21.
 *      Author: sipka
 */

#include <framework/geometry/Matrix.h>
#include <framework/animation/PropertyAnimator.h>
#include <framework/io/key/KeyEvent.h>
#include <framework/io/touch/TouchEvent.h>
#include <framework/layer/LayerGroup.h>
#include <framework/render/Renderer.h>
#include <framework/scene/Scene.h>
#include <framework/utils/BasicListener.h>
#include <gen/types.h>
#include <sapphire/DifficultySelectorLayer.h>
#include <sapphire/LevelSelectorLayer.h>
#include <sapphire/SapphireScene.h>
#include <sapphire/levelrender/LevelDrawer3D.h>
#include <sapphire/SapphireScene.h>
#include <sapphire/dialogs/OverallStatisticsDialog.h>
#include <cmath>
#include <stdio.h>
#include <cstdio>

namespace userapp {
using namespace rhfw;

#define TOTAL_VIEW_INDEX (((int) SapphireDifficulty::Unrated) + 1)

#define SET_DIFFICULTY(diff) \
	icons[(unsigned int) SapphireDifficulty::diff] = getAnimation(difficultyToAnimation(SapphireDifficulty::diff)); \
	labels[(unsigned int) SapphireDifficulty::diff] = difficultyToString(SapphireDifficulty::diff); \
	colors[(unsigned int) SapphireDifficulty::diff] = difficultyToColor(SapphireDifficulty::diff); \
	selectedColors[(unsigned int) SapphireDifficulty::diff] = difficultyToSelectedColor(SapphireDifficulty::diff);

#define DIFFICULTY_TITLE "Difficulty"

DifficultySelectorLayer::DifficultySelectorLayer(SapphireUILayer* parent)
		: SapphireUILayer(parent) {
	SET_DIFFICULTY(Tutorial);
	SET_DIFFICULTY(Simple);
	SET_DIFFICULTY(Easy);
	SET_DIFFICULTY(Moderate);
	SET_DIFFICULTY(Normal);
	SET_DIFFICULTY(Tricky);
	SET_DIFFICULTY(Tough);
	SET_DIFFICULTY(Difficult);
	SET_DIFFICULTY(Hard);
	SET_DIFFICULTY(M_A_D_);
	SET_DIFFICULTY(Unrated);

	icons[TOTAL_VIEW_INDEX] = getAnimation(ResIds::build::sipka_rh_texture_convert::box);
	labels[TOTAL_VIEW_INDEX] = "All levels";
	colors[TOTAL_VIEW_INDEX] = difficultyToColor(SapphireDifficulty::_count_of_entries);
	selectedColors[TOTAL_VIEW_INDEX] = difficultyToSelectedColor(SapphireDifficulty::_count_of_entries);

	dualTutorialIcon = getAnimation(ResIds::build::sipka_rh_texture_convert::_2man_anim);

	this->returnText = "Return to difficulty selector";
}
DifficultySelectorLayer::~DifficultySelectorLayer() {
	delete drawer3D;
}

template<>
bool DifficultySelectorLayer::isArrowVisible<DifficultySelectorLayer::Arrow::LEFT>() {
	auto currentpage = lastSelected / pageCapacity;
	return pageCapacity < DIFFICULTY_VIEW_COUNT && currentpage > 0;
}
template<>
bool DifficultySelectorLayer::isArrowVisible<DifficultySelectorLayer::Arrow::RIGHT>() {
	auto currentpage = lastSelected / pageCapacity;
	return pageCapacity < DIFFICULTY_VIEW_COUNT && currentpage < (DIFFICULTY_VIEW_COUNT - 1) / pageCapacity;
}

void DifficultySelectorLayer::drawImpl(float displayPercent) {
	auto ss = static_cast<SapphireScene*>(getScene());
	auto size = ss->getUiSize();
	renderer->setDepthTest(false);
	renderer->initDraw();

	float alpha = displayPercent * displayPercent;

	auto mvp = Matrix2D { }.setScreenDimension(size.pixelSize);

	if (backPressed) {
		drawRectangleColor(mvp, Color { SAPPHIRE_COLOR.rgb(), alpha }, backButtonPos);
	}
	drawSapphireTexture(mvp, backIconWhite, backPressed ? Color { 1, 1, 1, alpha } : Color { SAPPHIRE_COLOR.rgb(), alpha }, backButtonPos,
			Rectangle { 0, 0, 1, 1 });
	drawString(mvp, DIFFICULTY_TITLE, font, Color { SAPPHIRE_COLOR.rgb(), alpha }, Vector2F { backButtonPos.right,
			backButtonPos.middle().y() }, titleTextSize, Gravity::CENTER_VERTICAL | Gravity::LEFT);

	if (!ss->isLevelsLoaded()) {
		drawString(mvp, "Loading levels...", font, Color { SAPPHIRE_COLOR.rgb(), alpha }, size.pixelSize / 2.0f, titleTextSize / 2.0f,
				Gravity::CENTER);
		return;
	}

	auto currentpage = lastSelected / pageCapacity;

#ifdef SAPPHIRE_DUAL_PLAYER_AVAILABLE
	const char* plrcounttext = getPlayerCount() == 1 ? "Single player" : "Dual player";
	if (selectedDifficulty == SELECTED_PLAYERCOUNT || playerModeTouch != nullptr) {
		drawRectangleColor(mvp, Color { SAPPHIRE_COLOR.rgb(), alpha }, Rectangle { playerModeRect });
	}
	drawString(mvp, plrcounttext, font,
			Color {
					(selectedDifficulty == SELECTED_PLAYERCOUNT || playerModeTouch != nullptr ? Color { 1, 1, 1, alpha } : SAPPHIRE_COLOR).xyz(),
					alpha }, playerModeRect.middle(), playerModeTextSize, Gravity::CENTER);
#endif /* defined(SAPPHIRE_DUAL_PLAYER_AVAILABLE) */

	bool statsselected = selectedDifficulty == SELECTED_STATS || statsTouch != nullptr;
	if (statsselected) {
		drawRectangleColor(mvp, Color { SAPPHIRE_COLOR.rgb(), alpha }, Rectangle { statsRect });
	}
	drawSapphireTexture(mvp, statsIconWhite, statsselected ? Color { 1, 1, 1, alpha } : Color { SAPPHIRE_COLOR.rgb(), alpha }, statsRect,
			Rectangle { 0, 0, 1, 1 });

	if (pageCapacity < DIFFICULTY_VIEW_COUNT) {
		if (currentpage > 0) {
			//draw left arrow
			if (arrowTouch == Arrow::LEFT) {
				drawRectangleColor(mvp, Color { SAPPHIRE_COLOR.rgb(), alpha }, arrowLeftIconRect);
			}
			drawSapphireTexture(mvp, leftArrowWhite, arrowTouch == Arrow::LEFT ? Color { 1, 1, 1, alpha } : Color { SAPPHIRE_COLOR.rgb(),
																							alpha }, arrowLeftIconRect, Rectangle { 0, 0, 1,
					1 });
		}
		if (currentpage < (DIFFICULTY_VIEW_COUNT - 1) / pageCapacity) {
			//draw right arrow
			if (arrowTouch == Arrow::RIGHT) {
				drawRectangleColor(mvp, Color { SAPPHIRE_COLOR.rgb(), alpha }, arrowRightIconRect);
			}
			drawSapphireTexture(mvp, rightArrowWhite, arrowTouch == Arrow::RIGHT ? Color { 1, 1, 1, alpha } : Color { SAPPHIRE_COLOR.rgb(),
																							alpha }, arrowRightIconRect, Rectangle { 0, 0,
					1, 1 });
		}
	}

	SapphireDifficulty maxdif = ss->getMaxAllowedDifficulty();
	if (drawer3D != nullptr) {
		drawer3D->getManualColoredCommand().color = Color { 0.3f, 0.3f, 0.3f, 1 };

		mvp.multTranslate((pageOffset - (float) currentpage) * 2, 0);
		Size2F objsize { rects[0].width(), rects[0].width() };
		objsize *= 6.0f / 8.0f;
		for (int i = 0; i < DIFFICULTY_VIEW_COUNT; ++i) {
			bool selected = i == selectedDifficulty;
			bool diffplayable = i >= (int) SapphireDifficulty::_count_of_entries || i <= (int) maxdif
					|| (i == (int) SapphireDifficulty::Unrated && maxdif > SapphireDifficulty::Tutorial);
			Color dimcolor = diffplayable ? Color { 1, 1, 1, 1 } : Color { 0.3f, 0.3f, 0.3f, 1 };
			if (selected) {
				drawRectangleColor(mvp, dimcolor * Color { colors[i].xyz(), alpha }, rects[i]);
			}
			auto rect = Rectangle { rects[i].inset(rects[i].widthHeight() * 1.0f / 8.0f) };

			Matrix3D u_m;
			Matrix3D u_minv;
			Vector2F translate { (rects[i].middle().x() + (pageOffset - (float) currentpage) * size.pixelSize.width()) / objsize.width(),
					(size.pixelSize.height() - rects[i].bottom + rects[i].width() / 2.0f) / objsize.height() };
			u_m.setIdentity().multTranslate(translate.x(), translate.y(), 0.0f);
			u_minv.setTranslate(-translate.x(), -translate.y(), 0.0f);

			auto& cmd = diffplayable ? drawer3D->getDefaultColoredCommand() : drawer3D->getManualColoredCommand();
			unsigned int finishedlevelcount;
			unsigned int seenlevelcount;
			unsigned int totallevelcount;
			if (i < SAPPHIRE_DIFFICULTY_COUNT) {
				switch ((SapphireDifficulty) i) {
					case SapphireDifficulty::Tutorial: {
						auto& mesh = getPlayerCount() == 1 ? drawer3D->getMiner1Mesh() : drawer3D->getMiner2Mesh();
						if (iconAnimationValues[i] < 0.25f) {
							drawer3D->drawWalkingMiner(mesh, iconAnimationValues[i] / 0.25f, u_m, u_minv, SapphireDirection::Right,
									SapphireDirection::Down, iconAnimationValues[i] > 0.35f ? 1 : 0, cmd);
						} else {
							float tp = iconAnimationValues[i] > 0.5f ? iconAnimationValues[i] - 0.5f : iconAnimationValues[i] - 0.25f;
							drawer3D->drawWalkingMiner(mesh, tp / 0.25f, u_m, u_minv, SapphireDirection::Right, SapphireDirection::Right,
									iconAnimationValues[i] > 0.5f ? 0 : 1, cmd);
						}
						break;
					}
					case SapphireDifficulty::Simple: {
						float tp = iconAnimationValues[i] > 0.25f ? iconAnimationValues[i] - 0.25f : iconAnimationValues[i];
						drawer3D->drawRobot(tp * 4, u_m, u_minv, (unsigned int) (iconAnimationValues[i] / 0.25f), cmd);
						break;
					}
					case SapphireDifficulty::Easy: {
						drawer3D->drawSapphire(iconAnimationValues[i] / 0.7f, u_m, u_minv, cmd);
						break;
					}
					case SapphireDifficulty::Moderate: {
						drawer3D->drawEmerald(iconAnimationValues[i], u_m, u_minv, cmd);
						break;
					}
					case SapphireDifficulty::Normal: {
						if (iconAnimationValues[i] < 0.25f) {
							drawer3D->drawBuildingSwamp(1.0f - iconAnimationValues[i] / 0.25f, u_m, u_minv, cmd);
							drawer3D->drawDropHit(1.0f - iconAnimationValues[i] / 0.25f, u_m, u_minv, cmd);
						} else {
							float tp = iconAnimationValues[i] > 0.5f ? iconAnimationValues[i] - 0.5f : iconAnimationValues[i] - 0.25f;
							drawer3D->drawFallingDrop(tp / 0.25f, u_m, u_minv, iconAnimationValues[i] > 0.5f ? 0 : 1, cmd);
						}
						break;
					}
					case SapphireDifficulty::Tricky: {
						drawer3D->drawCitrine(iconAnimationValues[i] / 0.7f, u_m, u_minv, cmd);
						break;
					}
					case SapphireDifficulty::Tough: {
						if (iconAnimationValues[i] < 0.25f) {
							drawer3D->drawYamYam(sinf(iconAnimationValues[i] / 0.25f * 3.1415), iconAnimationValues[i] / 0.25f, u_m, u_minv,
									SapphireDirection::Undefined, oldYamYamDir, cmd);
						} else {
							drawer3D->drawYamYam(sinf((iconAnimationValues[i] - 0.25f) / 0.25f * 3.1415),
									(iconAnimationValues[i] - 0.25f) / 0.25f, u_m, u_minv, oldYamYamDir, yamYamDir, cmd);
						}
						break;
					}
					case SapphireDifficulty::Difficult: {
						drawer3D->drawRuby(iconAnimationValues[i] / 0.7f, u_m, u_minv, cmd);
						break;
					}
					case SapphireDifficulty::Hard: {
						drawer3D->drawBomb(iconAnimationValues[i], u_m, u_minv, cmd);
						break;
					}
					case SapphireDifficulty::M_A_D_: {
						if (iconAnimationValues[i] < 0.1f) {
							drawer3D->drawTickBomb(false, false, u_m, u_minv, cmd);
						} else {
							drawer3D->drawTickBomb(true, iconAnimationValues[i] < 0.1f + 0.25f, u_m, u_minv, cmd);
						}
						break;
					}
					case SapphireDifficulty::Unrated: {
						if (iconAnimationValues[i] < 0.5f) {
							drawer3D->drawBag(u_m, u_minv, cmd);
						} else if (iconAnimationValues[i] < 0.75f) {
							drawer3D->drawOpeningBag((iconAnimationValues[i] - 0.5f) / 0.25f, u_m, u_minv, cmd);
						} else if (iconAnimationValues[i] < 1.75f) {
							drawer3D->drawEmerald((iconAnimationValues[i] - 0.75f), u_m, u_minv, cmd);
						} else {
							drawer3D->drawOpeningBag(1.0f - (iconAnimationValues[i] - 1.75f) / 0.25f, u_m, u_minv, cmd);
						}
						break;
					}
					default: {
						THROW();
						break;
					}
				}
				finishedlevelcount = ss->getFinishedLevelCount(getPlayerCount(), (SapphireDifficulty) i);
				seenlevelcount = ss->getSeenLevelCount(getPlayerCount(), (SapphireDifficulty) i);
				totallevelcount = ss->getLevelCount(getPlayerCount(), (SapphireDifficulty) i);
			} else {

				switch (i) {
					case TOTAL_VIEW_INDEX: {
						finishedlevelcount = ss->getFinishedLevelCount(getPlayerCount());
						seenlevelcount = ss->getSeenLevelCount(getPlayerCount());
						totallevelcount = ss->getLevelCount(getPlayerCount());
						drawer3D->drawSafe(u_m, u_minv, cmd);
						break;
					}
					default: {
						THROW();
						break;
					}
				}
			}

			float labelsize = (rect.height() - rect.width()) * 2 / 3;

			drawString(mvp, labels[i], font,
					Color { (selected ? (diffplayable ? selectedColors[i] : Color { 1, 1, 1, 1 }) : colors[i] * dimcolor).rgb(), alpha },
					Vector2F { rects[i].middle().x(), rect.top }, labelsize, Gravity::CENTER);
			char buffer[256];
			sprintf(buffer, "%u/%u/%u", finishedlevelcount, seenlevelcount, totallevelcount);
			drawString(mvp, buffer, font,
					Color { (selected ? (diffplayable ? selectedColors[i] : Color { 1, 1, 1, 1 }) : colors[i] * dimcolor).rgb(), alpha * 0.7f },
					Vector2F { rects[i].middle().x(), rect.top + labelsize }, labelsize * 0.8f, Gravity::CENTER);
		}

		drawer3D->finishDrawing(alpha, 0.0f, objsize, size.pixelSize, Rectangle { 0, 0, 0, 0 }, 20,
				Vector2F { size.pixelSize } / objsize / 2.0f);
	}
	else {
		mvp.multTranslate((pageOffset - (float) currentpage) * 2, 0);
		for (int i = 0; i < DIFFICULTY_VIEW_COUNT; ++i) {
			bool selected = i == selectedDifficulty;
			bool diffplayable = i >= (int) SapphireDifficulty::_count_of_entries || i <= (int) maxdif
					|| (i == (int) SapphireDifficulty::Unrated && maxdif > SapphireDifficulty::Tutorial);
			Color dimcolor = diffplayable ? Color { 1, 1, 1, 1 } : Color { 0.3f, 0.3f, 0.3f, 1 };

			if (selected) {
				drawRectangleColor(mvp, dimcolor * Color { colors[i].xyz(), alpha }, rects[i]);
			}
			auto rect = Rectangle { rects[i].inset(rects[i].widthHeight() * 1.0f / 8.0f) };
			AutoResource<FrameAnimation>* icon = &icons[i];
			if ((SapphireDifficulty) i == SapphireDifficulty::Tutorial && getPlayerCount() == 2) {
				icon = &dualTutorialIcon;
			}
			auto& elem = iconAnimationValues[i] >= 1.0f ? (*icon)->getAtIndex(0) : (*icon)->getAtPercent(iconAnimationValues[i]);

			drawSapphireTexture(mvp, elem, Color { dimcolor.rgb(), alpha }, Rectangle { rect.left, rect.bottom - rect.width(), rect.right,
					rect.bottom }, Rectangle { elem.getPosition() });

			float labelsize = (rect.height() - rect.width()) * 2 / 3;

			unsigned int finishedlevelcount;
			unsigned int seenlevelcount;
			unsigned int totallevelcount;
			if (i < SAPPHIRE_DIFFICULTY_COUNT) {
				finishedlevelcount = ss->getFinishedLevelCount(getPlayerCount(), (SapphireDifficulty) i);
				seenlevelcount = ss->getSeenLevelCount(getPlayerCount(), (SapphireDifficulty) i);
				totallevelcount = ss->getLevelCount(getPlayerCount(), (SapphireDifficulty) i);
			} else {
				finishedlevelcount = ss->getFinishedLevelCount(getPlayerCount());
				seenlevelcount = ss->getSeenLevelCount(getPlayerCount());
				totallevelcount = ss->getLevelCount(getPlayerCount());
			}

			drawString(mvp, labels[i], font,
					Color { (selected ? (diffplayable ? selectedColors[i] : Color { 1, 1, 1, 1 }) : colors[i] * dimcolor).rgb(), alpha },
					Vector2F { rects[i].middle().x(), rect.top }, labelsize, Gravity::CENTER);
			char buffer[256];
			sprintf(buffer, "%u/%u/%u", finishedlevelcount, seenlevelcount, totallevelcount);
			drawString(mvp, buffer, font,
					Color { (selected ? (diffplayable ? selectedColors[i] : Color { 1, 1, 1, 1 }) : colors[i] * dimcolor).rgb(), alpha
							* 0.7f }, Vector2F { rects[i].middle().x(), rect.top + labelsize }, labelsize * 0.8f, Gravity::CENTER);
		}
	}
}
#define BREAK_ON_NOT_DOWN()  if (KeyEvent::instance.getAction() != KeyAction::DOWN) break
bool DifficultySelectorLayer::onKeyEventImpl() {
	switch (KeyEvent::instance.getKeycode()) {
		case KeyCode::KEY_GAMEPAD_DPAD_LEFT:
		case KeyCode::KEY_DIR_LEFT: {
			BREAK_ON_NOT_DOWN();

			if (selectedDifficulty < 0) {
#ifdef SAPPHIRE_DUAL_PLAYER_AVAILABLE
				if(selectedDifficulty == SELECTED_PLAYERCOUNT) {
					addToPlayerCount(-1);
				} else
#endif /* defined(SAPPHIRE_DUAL_PLAYER_AVAILABLE) */
				{
					setSelectedDifficulty(lastSelected);
				}
			} else {
				const auto& pos = positions[selectedDifficulty];
				unsigned int page = selectedDifficulty / pageCapacity;

				if(pos.x() % pageSize.width() == 0) {
					if(page > 0) {
						//will go to new page
						setSelectedDifficulty(selectedDifficulty - (pageCapacity - pageSize.width() + 1));
						pageOffset += -1.0f;
						startPageAnimator();
					} else {
						//first page
						if(pageCapacity >= DIFFICULTY_VIEW_COUNT) {
							int ndif = selectedDifficulty + pageSize.width() - 1;
							if(ndif >= DIFFICULTY_VIEW_COUNT) {
								ndif = DIFFICULTY_VIEW_COUNT - 1;
							}
							setSelectedDifficulty(ndif);
						}
					}
				} else {
					setSelectedDifficulty(selectedDifficulty - 1);
				}
			}
			break;
		}
		case KeyCode::KEY_GAMEPAD_DPAD_RIGHT:
		case KeyCode::KEY_DIR_RIGHT: {
			BREAK_ON_NOT_DOWN();

			if(selectedDifficulty < 0) {
#ifdef SAPPHIRE_DUAL_PLAYER_AVAILABLE
				if(selectedDifficulty == SELECTED_PLAYERCOUNT) {
					addToPlayerCount(1);
				} else
#endif /* defined(SAPPHIRE_DUAL_PLAYER_AVAILABLE) */
				{
					setSelectedDifficulty(lastSelected);
				}
			} else {
				const auto& pos = positions[selectedDifficulty];
				unsigned int page = selectedDifficulty / pageCapacity;

				if(pos.x() % pageSize.width() == pageSize.width() - 1) {
					//will go to new page
					if((page + 1) * pageCapacity < DIFFICULTY_VIEW_COUNT) {
						//next page has element
						int ndif = pageCapacity * (page + 1) + pageSize.width() * pos.y();
						if(ndif >= DIFFICULTY_VIEW_COUNT) {
							//next page is not full
							auto xmod = DIFFICULTY_VIEW_COUNT % pageSize.width();
							ndif = DIFFICULTY_VIEW_COUNT - (xmod == 0 ? pageSize.width() : xmod);
						}
						setSelectedDifficulty(ndif);
						pageOffset += 1.0f;
						startPageAnimator();
					} else if(pageCapacity >= DIFFICULTY_VIEW_COUNT) {
						//stay on page
						setSelectedDifficulty(selectedDifficulty - (pageSize.width() - 1));
					}
				} else {
					int ndif = selectedDifficulty + 1;
					if(ndif >= DIFFICULTY_VIEW_COUNT) {
						if(pos.y() == 0) {
							--ndif;
						} else {
							ndif -= pageSize.width();
						}
					}
					setSelectedDifficulty(ndif);
				}
			}
			break;
		}
		case KeyCode::KEY_GAMEPAD_DPAD_DOWN:
		case KeyCode::KEY_DIR_DOWN: {
			BREAK_ON_NOT_DOWN();
			if(selectedDifficulty < 0) {
				if(selectedDifficulty == SELECTED_STATS) {
					setSelectedDifficulty(SELECTED_PLAYERCOUNT);
				} else {
					setSelectedDifficulty(lastSelected);
				}
			} else {
				unsigned int page = selectedDifficulty / pageCapacity;
				int ndif = selectedDifficulty + pageSize.width();
				if(ndif / pageCapacity != page) {
					//would arrive on new page
					ndif -= pageCapacity;
				} else if(ndif >= DIFFICULTY_VIEW_COUNT) {
					//we are on a partial page
					if(ndif - (ndif % pageSize.width()) >= DIFFICULTY_VIEW_COUNT) {
						ndif -= pageSize.width();
					} else {
						//we are in a empty row
						ndif = DIFFICULTY_VIEW_COUNT - 1;
					}
				}
				setSelectedDifficulty(ndif);
			}
			break;
		}
		case KeyCode::KEY_GAMEPAD_DPAD_UP:
		case KeyCode::KEY_DIR_UP: {
			BREAK_ON_NOT_DOWN();
			if(selectedDifficulty < 0) {
				if(selectedDifficulty == SELECTED_PLAYERCOUNT) {
					setSelectedDifficulty(SELECTED_STATS);
				} else if(selectedDifficulty != SELECTED_STATS) {
					setSelectedDifficulty(lastSelected);
				}
			} else {
				const auto& pos = positions[selectedDifficulty];
				unsigned int page = selectedDifficulty / pageCapacity;
				int ndif = selectedDifficulty - pageSize.width();
				if(ndif / pageCapacity != page) {
#ifdef SAPPHIRE_DUAL_PLAYER_AVAILABLE
					setSelectedDifficulty(SELECTED_PLAYERCOUNT);
					break;
#else
					setSelectedDifficulty(SELECTED_STATS);
					break;
#endif /* defined(SAPPHIRE_DUAL_PLAYER_AVAILABLE) */
					//would arrive on new page
					ndif += pageCapacity;
				}
				/*no else here, because we added pageCapacity*/
				if(ndif >= DIFFICULTY_VIEW_COUNT) {
					auto emptyrows = (pageCapacity - DIFFICULTY_VIEW_COUNT % pageCapacity) / pageSize.width();
					ndif = (page + 1) * pageCapacity - pageSize.width() + pos.x() - emptyrows * pageSize.width();
					if(ndif >= DIFFICULTY_VIEW_COUNT) {
						ndif -= pageSize.width();
					}
				}
				setSelectedDifficulty(ndif);
			}
			break;
		}
		case KeyCode::KEY_GAMEPAD_RIGHT_SHOULDER:
		case KeyCode::KEY_PAGE_DOWN: {
			BREAK_ON_NOT_DOWN();
			if (isArrowVisible<Arrow::RIGHT>()) {
				auto diff = lastSelected;
				setSelectedDifficulty(min((int)DIFFICULTY_VIEW_COUNT - 1, diff + (int)pageCapacity));
				pageOffset += 1.0f;
				startPageAnimator();
			}
			break;
		}
		case KeyCode::KEY_GAMEPAD_LEFT_SHOULDER:
		case KeyCode::KEY_PAGE_UP: {
			BREAK_ON_NOT_DOWN();
			if (isArrowVisible<Arrow::LEFT>()) {
				auto diff = lastSelected;
				setSelectedDifficulty(max(diff - (int)pageCapacity, 0));
				pageOffset += -1.0f;
				startPageAnimator();
			}
			break;
		}
		case KeyCode::KEY_GAMEPAD_A:
		case KeyCode::KEY_ENTER: {
			BREAK_ON_NOT_DOWN();
			if(selectedDifficulty >= 0) {
				onSelectDifficulty(selectedDifficulty);
			}
#ifdef SAPPHIRE_DUAL_PLAYER_AVAILABLE
			else if(selectedDifficulty == SELECTED_PLAYERCOUNT) {
				addToPlayerCount(1);
			}
#endif /* defined(SAPPHIRE_DUAL_PLAYER_AVAILABLE) */
			else if(selectedDifficulty == SELECTED_STATS) {
				showLifetimeStatDialog();
			}
			break;
		}
		case KeyCode::KEY_TAB: {
#ifdef SAPPHIRE_DUAL_PLAYER_AVAILABLE
			BREAK_ON_NOT_DOWN();
			addToPlayerCount(1);
#endif /* defined(SAPPHIRE_DUAL_PLAYER_AVAILABLE) */
			break;
		}
		default: {
			return SapphireUILayer::onKeyEventImpl();
		}
	}
	return true;
}

bool DifficultySelectorLayer::touchImpl() {
	if (!showing || !gainedInput || TouchEvent::instance.getPointerCount() > 1 || TouchEvent::instance.getAction() == TouchAction::CANCEL) {
		arrowTouch = Arrow::NONE;
		backPressed = false;
		clearSelected();

		statsTouch = nullptr;
#ifdef SAPPHIRE_DUAL_PLAYER_AVAILABLE
		playerModeTouch = nullptr;
#endif /* defined(SAPPHIRE_DUAL_PLAYER_AVAILABLE) */
		return true;
	}

	switch (TouchEvent::instance.getAction()) {
		case TouchAction::DOWN: {
			clearSelected();
			arrowTouch = Arrow::NONE;
			auto touchpos = TouchEvent::instance.getAffectedPointer()->getPosition();
			if (backButtonPos.isInside(touchpos)) {
				backPressed = true;
			} else if (isArrowVisible<Arrow::LEFT>() && arrowLeft.isInside(touchpos)) {
				arrowTouch = Arrow::LEFT;
				break;
			} else if (isArrowVisible<Arrow::RIGHT>() && arrowRight.isInside(touchpos)) {
				arrowTouch = Arrow::RIGHT;
				break;
			}
#ifdef SAPPHIRE_DUAL_PLAYER_AVAILABLE
			else if (playerModeRect.isInside(touchpos)) {
				playerModeTouch = TouchEvent::instance.getAffectedPointer();
				break;
			}
#endif /* defined(SAPPHIRE_DUAL_PLAYER_AVAILABLE) */
			else if (statsRect.isInside(touchpos)) {
				statsTouch = TouchEvent::instance.getAffectedPointer();
				break;
			} else {
				arrowTouch = Arrow::NONE;
				auto currentpage = lastSelected / pageCapacity;
				touchpos -= Vector2F { (pageOffset - (float) currentpage)
						* static_cast<SapphireScene*>(getScene())->getUiSize().pixelSize.width(), 0 };
				for (int i = 0; i < DIFFICULTY_VIEW_COUNT; ++i) {
					if (rects[i].isInside(touchpos)) {
						setSelectedDifficulty(i);
						break;
					}
				}
			}

			break;
		}
		case TouchAction::MOVE_UPDATE: {
#ifdef SAPPHIRE_DUAL_PLAYER_AVAILABLE
			if (playerModeTouch != nullptr && !playerModeRect.isInside(playerModeTouch->getPosition())) {
				playerModeTouch = nullptr;
			}
#endif /* defined(SAPPHIRE_DUAL_PLAYER_AVAILABLE) */
			if (statsTouch != nullptr && !statsRect.isInside(statsTouch->getPosition())) {
				statsTouch = nullptr;
			}
			auto touchpos = TouchEvent::instance.getAffectedPointer()->getPosition();
			if (backPressed && !backButtonPos.isInside(touchpos)) {
				backPressed = false;
			} else if ((arrowTouch == Arrow::LEFT && !arrowLeft.isInside(touchpos))
					|| (arrowTouch == Arrow::RIGHT && !arrowRight.isInside(touchpos))) {
				arrowTouch = Arrow::NONE;
			} else {
				auto currentpage = lastSelected / pageCapacity;
				touchpos -= Vector2F { (pageOffset - (float) currentpage)
						* static_cast<SapphireScene*>(getScene())->getUiSize().pixelSize.width(), 0 };
				if (selectedDifficulty >= 0 && !rects[selectedDifficulty].isInside(touchpos)) {
					clearSelected();
				}
			}
			break;
		}
		case TouchAction::UP: {
#ifdef SAPPHIRE_DUAL_PLAYER_AVAILABLE
			if (playerModeTouch == TouchEvent::instance.getAffectedPointer()) {
				//TODO if no hardware keyboard detected, display warning
				addToPlayerCount(1);
				playerModeTouch = nullptr;
				break;
			}
#endif /* defined(SAPPHIRE_DUAL_PLAYER_AVAILABLE) */
			if (statsTouch == TouchEvent::instance.getAffectedPointer()) {
				//TODO if no hardware keyboard detected, display warning
				showLifetimeStatDialog();
				statsTouch = nullptr;
				break;
			}
			auto currentpage = lastSelected / pageCapacity;
			if (backPressed) {
				backPressed = false;
				dismiss();
			} else if (arrowTouch == Arrow::LEFT) {
				setSelectedDifficulty(-1);
				lastSelected = (currentpage - 1) * pageCapacity;
				pageOffset += -1.0f;
				startPageAnimator();
				arrowTouch = Arrow::NONE;
			} else if (arrowTouch == Arrow::RIGHT) {
				lastSelected = (currentpage + 1) * pageCapacity;
				setSelectedDifficulty(-1);
				pageOffset += 1.0f;
				startPageAnimator();
				arrowTouch = Arrow::NONE;
			} else if (selectedDifficulty >= 0) {
				onSelectDifficulty(selectedDifficulty);
				clearSelected();
			}
			break;
		}
		default: {
			break;
		}
	}

	return true;
}

static Size2UI getMaxPagesForSize(const Size2F& minsize, const Rectangle& rect, Size2F* elementsizeout) {
	Size2UI result = (Size2UI) (rect.widthHeight() / minsize);

	float ratio = minsize.width() / minsize.height();
	float rectratio = rect.width() / rect.height();

	if (result.width() == 0) {
		result.width() = 1;
		if (result.height() == 0) {
			result.height() = 1;
			if (ratio > rectratio) {
				elementsizeout->width() = rect.width();
				elementsizeout->height() = rect.width() / rectratio;
			} else {
				elementsizeout->width() = rect.height() * rectratio;
				elementsizeout->height() = rect.height();
			}
		} else {
			elementsizeout->width() = rect.width();
			elementsizeout->height() = rect.width() / ratio;
		}
	} else {
		if (result.height() == 0) {
			result.height() = 1;
			elementsizeout->width() = rect.height() * ratio;
			elementsizeout->height() = rect.height();
		} else {
			*elementsizeout = minsize;
		}
	}
	return result;
}

bool DifficultySelectorLayer::showLockedLevelDialog(SapphireUILayer* parent, SapphireDifficulty difficulty) {
	//no longer used
	return false;
}
void DifficultySelectorLayer::onSelectDifficulty(int id) {
	auto* ss = static_cast<SapphireScene*>(getScene());
	if (id == TOTAL_VIEW_INDEX) {
		LevelSelectorLayer* layer = new LevelSelectorLayer(this);
		layer->setupDefaultPartitions(ss, SapphireDifficulty::_count_of_entries, getPlayerCount(), labels[TOTAL_VIEW_INDEX]);
		layer->show(getScene());
	} else {
		if (!showLockedLevelDialog(this, (SapphireDifficulty) id)) {
			LevelSelectorLayer* layer = new LevelSelectorLayer(this, (SapphireDifficulty) id, getPlayerCount());
			layer->show(getScene());
		}
	}
}

void DifficultySelectorLayer::setSelectedDifficulty(int index) {
	if (index == selectedDifficulty) {
		return;
	}

	if (this->selectedDifficulty >= 0) {
		startIconAnimation(this->selectedDifficulty, 0.0f);
	}

	this->selectedDifficulty = index;
	if (index >= 0) {
		this->lastSelected = index;
	}
	if (this->selectedDifficulty >= 0) {
		if (drawer3D == nullptr) {
			//if 3D icons, then autoanimating in drawimpl
			startIconAnimation(this->selectedDifficulty, 1.0f);
		}
	}

}

void DifficultySelectorLayer::startIconAnimation(int index, float target) {
	auto& animlink = iconAnimations[index];
	animlink.kill();
	long long duration = (long long) (250 * fabsf(iconAnimationValues[index] - target));
	if (index == 0) {
		//slow down tutorial
		duration *= 3;
	}
	auto* anim = new PropertyAnimator<float>(iconAnimationValues[index], target, core::MonotonicTime::getCurrent(), core::time_millis {
			duration });
	animlink.link(*anim);
	anim->start();
}
void DifficultySelectorLayer::startPageAnimator() {
	class PageAnimator: public PropertyAnimator<float> {
	private:
	public:
		float a = 1.0f;
		float b = 0.0f;
		using PropertyAnimator<float>::PropertyAnimator;

		void onProgress(const core::time_millis& progress) override {
			const float percent = 1 - (float) ((progress - starttime) / duration);
			float eq = 1 - (a * percent * percent + b * percent);
			PropertyAnimator<float>::onProgress(starttime + core::time_millis { (long long) ((eq * (float) (long long) duration)) });
		}
	};
	PageAnimator* curanim = static_cast<PageAnimator*>(pageAnimator.get());
	auto start = core::MonotonicTime::getCurrent();
	core::time_millis duration { 250 };
	if (fabsf(pageOffset) < 1.0f) {
		duration = core::time_millis { (long long) (250 * fabsf(pageOffset)) };
	}
	auto* anim = new PageAnimator(pageOffset, 0.0f, start, duration);
	if (curanim != nullptr) {
		const float percent = (float) ((start - curanim->getStartTime()) / curanim->getDuration());

		anim->b = (2 * curanim->a * percent + curanim->b);
		anim->a = -anim->b + 1;

		pageAnimator.kill();
	}
	pageAnimator.link(anim);
	anim->start();
}

#ifdef SAPPHIRE_DUAL_PLAYER_AVAILABLE
void DifficultySelectorLayer::addToPlayerCount(int count) {
	playerCount = (playerCount + count) % MAX_PLAYER_COUNT;
}
#endif /* defined(SAPPHIRE_DUAL_PLAYER_AVAILABLE) */

void DifficultySelectorLayer::sizeChanged(const rhfw::core::WindowSize& size) {
	static_assert(DIFFICULTY_VIEW_COUNT > 0, "difficulty count is zero");

	const Size2F defaultsize { size.toPixelsX(3.2f), size.toPixelsY(4.5f) };

	Size2F arrowpadding = size.toPixels(Size2F { min(1.5f, size.pixelSize.width() / 10.0f), 0.0f });

	float titleheight = size.pixelSize.height() * SAPPHIRE_TITLE_PERCENT;
	if (titleheight > size.toPixelsY(3.5f)) {
		titleheight = size.toPixelsY(3.5f);
	}
	float titlew = titleheight + font->measureText(DIFFICULTY_TITLE, titleheight);
	if (titlew > size.pixelSize.width() * 7.0f / 8.0f) {
		titleheight = titleheight / titlew * size.pixelSize.width() * 7.0f / 8.0f;
		titlew = size.pixelSize.width() * 7.0f / 8.0f;
	}
	titleTextSize = titleheight;
	backButtonPos = Rectangle { (size.pixelSize.width() - titlew) / 2.0f, 0, (size.pixelSize.width() - titlew) / 2.0f + titleheight,
			titleheight };

	Rectangle rect { arrowpadding.width(), arrowpadding.height() + titleheight, (float) size.pixelSize.width() - arrowpadding.width(),
			(float) size.pixelSize.height() - arrowpadding.height() };

#ifdef SAPPHIRE_DUAL_PLAYER_AVAILABLE
	playerModeTextSize = titleTextSize / 2.0f;
	float plrmodetextwidth = font->measureText("Single player", playerModeTextSize);
	playerModeRect = {size.pixelSize.width() / 2.0f - plrmodetextwidth / 2.0f * 1.2f, backButtonPos.bottom,
		size.pixelSize.width() / 2.0f + plrmodetextwidth / 2.0f * 1.2f, backButtonPos.bottom + playerModeTextSize * 1.2f};

	rect.top += playerModeRect.height();
#endif /* defined(SAPPHIRE_DUAL_PLAYER_AVAILABLE) */

	statsRect = Rectangle { 0, 0, backButtonPos.width(), backButtonPos.height() }.translate(
			Vector2F { size.pixelSize.width() - backButtonPos.width(), 0 });
	statsRect = statsRect.inset(statsRect.widthHeight() * 0.175f);

	pageSize = getMaxPagesForSize(defaultsize, rect, &elementPixelSize);

	int pagecapacity = pageSize.height() * pageSize.width();
	unsigned int idealwidth = (unsigned int) (sqrtf(DIFFICULTY_VIEW_COUNT));
	if (idealwidth * idealwidth < DIFFICULTY_VIEW_COUNT) {
		++idealwidth;
	}
	while (pageSize.width() > 1 && pageSize.width() > idealwidth && pagecapacity - pageSize.height() >= DIFFICULTY_VIEW_COUNT) {
		--pageSize.width();
		pagecapacity -= pageSize.height();
	}
	unsigned int decrementheight = (DIFFICULTY_VIEW_COUNT + pageSize.width()) / pageSize.width();
	if (pageSize.height() > decrementheight) {
		pageSize.height() = decrementheight;
		pagecapacity = pageSize.height() * pageSize.width();
	}
	this->pageCapacity = pagecapacity;

	Size2F pagepadding = (rect.widthHeight() - elementPixelSize * pageSize);
	Size2F rectpadding = size.pixelSize - rect.widthHeight();
	if (pagecapacity >= DIFFICULTY_VIEW_COUNT) {
		pagepadding.height() = 0;
	} else {
		//assign arrows
		arrowLeft = Rectangle { 0, rect.top, rect.left + pagepadding.width() / 2.0f, rect.bottom };
		arrowRight = Rectangle { rect.right - pagepadding.width() / 2.0f, rect.top, (float) size.pixelSize.width(), rect.bottom };

		Rectangle maxIcon { 0, 0, size.toPixelsX(2.0f), size.toPixelsY(2.0f) };
		if (maxIcon.width() > arrowLeft.width()) {
			float diff = maxIcon.width() - arrowLeft.width();
			maxIcon.right -= diff;
			maxIcon.bottom -= diff;
		}
		if (maxIcon.height() > arrowLeft.height()) {
			float diff = maxIcon.height() - arrowLeft.height();
			maxIcon.right -= diff;
			maxIcon.bottom -= diff;
		}

		arrowLeftIconRect = Rectangle { maxIcon.translate(arrowLeft.middle() - maxIcon.widthHeight() / 2.0f) };
		arrowRightIconRect = Rectangle { maxIcon.translate(arrowRight.middle() - maxIcon.widthHeight() / 2.0f) };
	}

	Vector2F rectsmiddle { 0, 0 };
	for (int i = 0; i < DIFFICULTY_VIEW_COUNT; ++i) {
		int page = i / pagecapacity;
		Vector2UI pagev { i / pagecapacity, 0 };

		positions[i] = Vector2UI { i % pageSize.width() + page * pageSize.width(), (i % pagecapacity) / pageSize.width() };
		rects[i] = Rectangle { //
				pagepadding / 2.0f + rect.leftTop() + elementPixelSize * positions[i] + pagepadding * pagev + rectpadding * pagev, //
				pagepadding / 2.0f + rect.leftTop() + elementPixelSize * (positions[i] + 1) + pagepadding * pagev + rectpadding * pagev, //
				};
		if (i < pageCapacity) {
			rectsmiddle += rects[i].middle();
		}
	}
	rectsmiddle /= min((unsigned int) DIFFICULTY_VIEW_COUNT, pageCapacity);

	if (drawer3D != nullptr) {
		Size2F objsize { rects[0].width(), rects[0].width() };
		objsize *= 6.0f / 8.0f;
		Vector2F mid { (rectsmiddle.x()) / objsize.width(), (size.pixelSize.height() - rectsmiddle.y()) / objsize.height() + 0.2f };
		drawer3D->setFixedLighting(mid);
	}
}

void DifficultySelectorLayer::onTimeChanged(const core::time_millis& time, const core::time_millis& previous) {
#define ADVANCE_SELECTED(period) \
	iconAnimationValues[selectedDifficulty] += sec; \
	while(iconAnimationValues[selectedDifficulty] > (period)){\
		iconAnimationValues[selectedDifficulty] -= (period);\
	}

	long long ms = (long long) (core::time_millis { time - previous });
	if (ms > 2000) {
		ms = 2000 + ms % 2000;
	}
	float sec = ms / 1000.0f;
	switch ((SapphireDifficulty) selectedDifficulty) {
		case SapphireDifficulty::Tutorial: {
			iconAnimationValues[selectedDifficulty] += sec;
			while (iconAnimationValues[selectedDifficulty] > 0.75f) {
				iconAnimationValues[selectedDifficulty] -= 0.5f;
			}
			break;
		}
		case SapphireDifficulty::Simple: {
			ADVANCE_SELECTED(0.5f);
			break;
		}
		case SapphireDifficulty::Easy: {
			ADVANCE_SELECTED(0.7f);
			break;
		}
		case SapphireDifficulty::Moderate: {
			ADVANCE_SELECTED(1.0f);
			break;
		}
		case SapphireDifficulty::Normal: {
			iconAnimationValues[selectedDifficulty] += sec;
			while (iconAnimationValues[selectedDifficulty] > 0.75f) {
				iconAnimationValues[selectedDifficulty] -= 0.5f;
			}
			break;
		}
		case SapphireDifficulty::Tricky: {
			ADVANCE_SELECTED(0.7f);
			break;
		}
		case SapphireDifficulty::Tough: {
			iconAnimationValues[selectedDifficulty] += sec;
			while (iconAnimationValues[selectedDifficulty] > 0.5f) {
				oldYamYamDir = yamYamDir;
				yamYamDir = (SapphireDirection) randomer.next(4);
				iconAnimationValues[selectedDifficulty] -= 0.25f;
			}
			break;
		}
		case SapphireDifficulty::Difficult: {
			ADVANCE_SELECTED(0.7f);
			break;
		}
		case SapphireDifficulty::Hard: {
			ADVANCE_SELECTED(2.0f);
			break;
		}
		case SapphireDifficulty::M_A_D_: {
			iconAnimationValues[selectedDifficulty] += sec;
			while (iconAnimationValues[selectedDifficulty] > 0.6f) {
				iconAnimationValues[selectedDifficulty] -= 0.5f;
			}
			break;
		}
		case SapphireDifficulty::Unrated: {
			ADVANCE_SELECTED(2.0f);
			break;
		}
		default: {
			break;
		}
	}
	for (int i = 0; i < DIFFICULTY_VIEW_COUNT; ++i) {
		bool selected = i == selectedDifficulty;
		if (!selected && iconAnimationValues[i] > 0.0f) {
			iconAnimationValues[i] -= sec;
			if (iconAnimationValues[i] < 0.0f) {
				iconAnimationValues[i] = 0.0f;
			}
		}
	}
}

void DifficultySelectorLayer::showLifetimeStatDialog() {
	DialogLayer* dialog = new OverallStatisticsDialog(this, static_cast<SapphireScene*>(getScene())->getTotalStatistics());
	dialog->showDialog(getScene());
}

void DifficultySelectorLayer::setScene(Scene* scene) {
	SapphireUILayer::setScene(scene);
	if (static_cast<SapphireScene*>(scene)->getSettings().is3DArtStyle()) {
		scene->getWindow()->foregroundTimeListeners += *this;
		drawer3D = new LevelDrawer3D();
		drawer3D->setOrthographic(true);
	}
	static_cast<SapphireScene*>(scene)->settingsChangedListeners += *this;
	randomer.setSeed((unsigned int) core::time_millis { core::MonotonicTime::getCurrent() });
	oldYamYamDir = (SapphireDirection) randomer.next(4);
	yamYamDir = (SapphireDirection) randomer.next(4);
}

void DifficultySelectorLayer::onSettingsChanged(const SapphireSettings& settings) {
	if (settings.is3DArtStyle()) {
		if (drawer3D == nullptr) {
			getScene()->getWindow()->foregroundTimeListeners += *this;
			drawer3D = new LevelDrawer3D();
			drawer3D->setOrthographic(true);
		}
	} else {
		if (drawer3D != nullptr) {
			TimeListener::unsubscribe();
			delete drawer3D;
			drawer3D = nullptr;
		}
	}
}

} // namespace userapp

