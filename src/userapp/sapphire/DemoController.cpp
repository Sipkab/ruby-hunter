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
 * DemoController.cpp
 *
 *  Created on: 2016. dec. 20.
 *      Author: sipka
 */

#include <sapphire/DemoController.h>
#include <sapphire/SapphireScene.h>
#include <sapphire/level/Demo.h>
#include <sapphire/sapphireconstants.h>
#include <framework/io/key/KeyEvent.h>
#include <framework/animation/PropertyAnimator.h>

#include <gen/log.h>

namespace userapp {

static const char* ControlLabels[] { "Fast backward", "Backward", "Slow backward", "Pause", "Slow forward", "Forward", "Fast forward" };

#define CONTROLS_BUTTON_LABEL "Demo controls"

class ControlsAnimator: public PropertyAnimator<float> {
protected:
public:
	ControlsAnimator(bool visible, float& value, core::time_millis start)
			: PropertyAnimator<float>(value, visible ? 1.0f : 0.0f, start, core::time_millis(200)) {
	}
};

DemoController::DemoController(Level* level)
		: level(level) {
	controlElems[(unsigned int) ControlIndex::PAUSE] = &vcratlas->getAtIndex((unsigned int) VcrAtlas::CONTROL_PAUSE);
	controlElems[(unsigned int) ControlIndex::PLAY] = &vcratlas->getAtIndex((unsigned int) VcrAtlas::CONTROL_FORWARD);
	controlElems[(unsigned int) ControlIndex::SLOW_PLAY] = &vcratlas->getAtIndex((unsigned int) VcrAtlas::CONTROL_SLOW_FORWARD);
	controlElems[(unsigned int) ControlIndex::FAST_PLAY] = &vcratlas->getAtIndex((unsigned int) VcrAtlas::CONTROL_FAST_FORWARD);
	controlElems[(unsigned int) ControlIndex::BACK] = &vcratlas->getAtIndex((unsigned int) VcrAtlas::CONTROL_BACK);
	controlElems[(unsigned int) ControlIndex::SLOW_BACK] = &vcratlas->getAtIndex((unsigned int) VcrAtlas::CONTROL_SLOW_BACK);
	controlElems[(unsigned int) ControlIndex::FAST_BACK] = &vcratlas->getAtIndex((unsigned int) VcrAtlas::CONTROL_FAST_BACK);

	controlIcons[(unsigned int) ControlIndex::PAUSE] = getAnimation(ResIds::build::sipka_rh_texture_convert::ic_pause_anim);
	controlIcons[(unsigned int) ControlIndex::PLAY] = getAnimation(ResIds::build::sipka_rh_texture_convert::ic_play60_anim);
	controlIcons[(unsigned int) ControlIndex::SLOW_PLAY] = getAnimation(ResIds::build::sipka_rh_texture_convert::ic_step_forward_anim);
	controlIcons[(unsigned int) ControlIndex::FAST_PLAY] = getAnimation(ResIds::build::sipka_rh_texture_convert::ic_fast_forward_anim);
	controlIcons[(unsigned int) ControlIndex::BACK] = getAnimation(ResIds::build::sipka_rh_texture_convert::ic_play60_anim);
	controlIcons[(unsigned int) ControlIndex::SLOW_BACK] = getAnimation(ResIds::build::sipka_rh_texture_convert::ic_step_forward_anim);
	controlIcons[(unsigned int) ControlIndex::FAST_BACK] = getAnimation(ResIds::build::sipka_rh_texture_convert::ic_fast_forward_anim);
}
DemoController::~DemoController() {
}

void DemoController::drawSpeedIndicator(const Matrix2D& mvp, float alpha, const Rectangle& rect, ControlIndex index, const Color& color,
		const Color& selcolor) {
	Color alphad = color;
	alphad.a() *= alpha;
	float insetw = rect.width() * 0.05f;
	float inseth = rect.height() * 0.05f;
	drawRectangleColor(mvp, alphad, rect);
	drawRectangleColor(mvp, Color { 0, 0, 0, alpha }, rect.inset(min(insetw, inseth)));
	Rectangle inside = rect.fitInto(Rectangle { 0, 0, 1, 1 });
	auto&& elem =
			index < ControlIndex::PAUSE ?
					controlIcons[(unsigned int) index]->getAtIndex(0).flippedY() : controlIcons[(unsigned int) index]->getAtIndex(0);
	drawSapphireTexture(mvp, elem.getTexture(), alphad, inside, elem.getPosition());
}

void DemoController::onSizeChanged(const core::WindowSize& size) {
	mvp.setScreenDimension(size.pixelSize.width(), size.pixelSize.height());

	hudHeight = size.toPixelsY(min(min(size.getPhysicalSize().height() / 18, 1.0f), size.getPhysicalSize().width() / 11.0f));

	float maxw = 0.0f;
	for (unsigned int i = 0; i < CONTROLS_COUNT; ++i) {
		float measured = font->measureText(ControlLabels[i], hudHeight);
		if (measured > maxw) {
			maxw = measured;
		}
	}
	//width of icon + padding
	maxw += hudHeight * 1.5f;
	maxw += size.toPixelsX(0.5f);
	float maximumcontrolswidth = size.pixelSize.width() * 0.75f;
	if (maxw > maximumcontrolswidth) {
		maxw = maximumcontrolswidth;
	}
	controlsItemsPerRow = maximumcontrolswidth / maxw;
	if (controlsItemsPerRow == 0) {
		controlsItemsPerRow = 1;
	}
	controlsRowCount = CONTROLS_COUNT / (controlsItemsPerRow);
	if (CONTROLS_COUNT % controlsItemsPerRow != 0) {
		++controlsRowCount;
	}
	while (controlsItemsPerRow > 1 && (controlsItemsPerRow - 1) * controlsRowCount >= CONTROLS_COUNT
			&& (controlsItemsPerRow - 1) * controlsRowCount < controlsItemsPerRow * controlsRowCount) {
		--controlsItemsPerRow;
	}

	paddings = Rectangle { hudHeight * controlsRowCount, hudHeight * controlsRowCount, hudHeight * controlsRowCount, hudHeight
			* controlsRowCount };

	float controlsAreaWidth = controlsItemsPerRow * maxw;

	controlAreaRectangle.left = (size.pixelSize.width() - controlsAreaWidth) / 2.0f;
	controlAreaRectangle.right = controlAreaRectangle.left + controlsAreaWidth;
	controlAreaRectangle.bottom = size.pixelSize.height();
	controlAreaRectangle.top = controlAreaRectangle.bottom - hudHeight * controlsRowCount;
	controlAreaBorderRectangle = controlAreaRectangle.inset(size.toPixels(Vector2F { -0.07f, -0.07f }));

	for (unsigned int i = 0; i < CONTROLS_COUNT; ++i) {
		controlsRects[i] = Rectangle { i % controlsItemsPerRow * maxw, i / controlsItemsPerRow * hudHeight, (i % controlsItemsPerRow + 1)
				* maxw, (i / controlsItemsPerRow + 1) * hudHeight }.translate(controlAreaRectangle.leftTop());
	}

	float controlstextw = font->measureText(CONTROLS_BUTTON_LABEL, hudHeight);
	controlButtonRect = Rectangle { size.pixelSize.width() / 2.0f - controlstextw / 2.0f, size.pixelSize.height() - hudHeight,
			size.pixelSize.width() / 2.0f + controlstextw / 2.0f, (float) size.pixelSize.height() }.inset(size.toPixelsX(-0.3f));
	//to fix middle translation introducet by inset call
	controlButtonRect.bottom = size.pixelSize.height();
}

void DemoController::draw(float turnpercent, float displaypercent) {
#ifdef SAPPHIRE_SCREENSHOT_MODE
	return;
#endif /* defined(SAPPHIRE_SCREENSHOT_MODE) */

	Color difcol = difficultyToColor(level->getInfo().difficulty);
	Color difselcol = difficultyToSelectedColor(level->getInfo().difficulty);

	int plrcount = level->getPlayerCount();
	Vector2F lefttop { hudHeight + -turnpercent * hudHeight, 0.0f };
	Vector2F rightbottom { 2 * hudHeight - turnpercent * hudHeight, hudHeight };

	int index = (((int) level->getTurn()) - 4) * plrcount;
	for (unsigned int pc = 0; pc < plrcount; ++pc) {
		Rectangle current { lefttop, rightbottom };
		for (int i = 0; i < 10; ++i) {
			float alpha = i == 9 ? displaypercent * turnpercent : (i == 0 ? (1 - turnpercent) : displaypercent);
			if ((unsigned int) (index + i * plrcount + pc) < demoSteps.length()) {
				char move = demoSteps[(unsigned int) (index + i * plrcount + pc)];
				SapphireControl c;
				SapphireDirection dir;
				DemoPlayer::decodeDemoMove(move, &dir, &c);
				switch (c) {
					case SapphireControl::Undefined: {
						auto& elem = vcratlas->getAtIndex((unsigned int) VcrAtlas::ACTION_NONE);
						drawSapphireTexture(mvp, elem, alpha, current, elem.getPosition());
						break;
					}
					case SapphireControl::Move: {
						auto& elem = vcratlas->getAtIndex((unsigned int) VcrAtlas::ACTION_MOVE + (unsigned int) dir);
						drawSapphireTexture(mvp, elem, alpha, current, elem.getPosition());
						break;
					}
					case SapphireControl::Take: {
						auto& elem = vcratlas->getAtIndex((unsigned int) VcrAtlas::ACTION_PICK + (unsigned int) dir);
						drawSapphireTexture(mvp, elem, alpha, current, elem.getPosition());
						break;
					}
					case SapphireControl::PutBomb: {
						auto& elem = vcratlas->getAtIndex((unsigned int) VcrAtlas::ACTION_PUTBOMB + (unsigned int) dir);
						drawSapphireTexture(mvp, elem, alpha, current, elem.getPosition());
						break;
					}
					default: {
						THROW() << "Unknown control " << c;
						break;
					}
				}
			} else if ((index + i * plrcount + (int) pc) >= 0) {
				if (!useRecordingIcons) {
					auto& elem = vcratlas->getAtIndex((unsigned int) VcrAtlas::ACTION_NONE);
					drawSapphireTexture(mvp, elem, alpha, current, elem.getPosition());
				}
			}
			current = Rectangle { current.translate(Vector2F { hudHeight, 0.0f }) };
		}

		auto& indicator = vcratlas->getAtIndex((unsigned int) VcrAtlas::CONTROL_FRAME);
		drawSapphireTexture(mvp, indicator, Color { difcol.rgb(), displaypercent }, Rectangle { Vector2F { hudHeight * 4, lefttop.y() },
				Vector2F { hudHeight + hudHeight * 4, rightbottom.y() } }, indicator.getPosition());

		lefttop.y() += hudHeight;
		rightbottom.y() += hudHeight;
	}
//	auto& stateelem = vcratlas->getAtIndex((unsigned int) playerAtlasIcon);
//	drawSapphireTexture(mvp, stateelem, displaypercent,
//			Rectangle { 0, (hudHeight * (plrcount - 1) / 2.0f), hudHeight, (hudHeight * (plrcount - 1) / 2.0f) + hudHeight },
//			stateelem.getPosition());

	drawSpeedIndicator(mvp, displaypercent, Rectangle { 0, 0, hudHeight, hudHeight * plrcount }, playerControlIndex, difcol, difselcol);

	if (controlButtonTouch != nullptr) {
		drawRectangleColor(mvp, Color { difcol.rgb(), displaypercent }, controlButtonRect);
	}
	drawString(mvp, CONTROLS_BUTTON_LABEL, font,
			controlButtonTouch != nullptr ?
					Color { difselcol.rgb(), (1 - displayAlpha) * displaypercent } :
					Color { difcol.rgb(), (1 - displayAlpha) * displaypercent }, controlButtonRect.middle(), hudHeight, Gravity::CENTER);

	if (displayAlpha > 0.0f) {
		drawRectangleColor(mvp, Color { difcol.rgb(), displayAlpha * displayAlpha * displayAlpha * displaypercent },
				controlAreaBorderRectangle);
		drawRectangleColor(mvp, Color { 0, 0, 0, displayAlpha * displaypercent }, controlAreaRectangle);
		for (unsigned int i = 0; i < CONTROLS_COUNT; ++i) {
			bool highlight = playerAtlasIcon == vcrIndexAtlas[i] || controlsTouch[i] != nullptr;
			if (highlight) {
				drawRectangleColor(mvp, Color { difcol.rgb(), displayAlpha * displaypercent }, controlsRects[i]);
			}
			drawSpeedIndicator(mvp, displayAlpha * displaypercent,
					Rectangle { controlsRects[i].left, controlsRects[i].top, controlsRects[i].left + hudHeight, controlsRects[i].bottom },
					(ControlIndex) i, difcol, difselcol);

//			auto& stateelem = vcratlas->getAtIndex((unsigned int) vcrIndexAtlas[i]);
//
//			drawSapphireTexture(mvp, stateelem, displayAlpha * displaypercent,
//					Rectangle { controlsRects[i].left, controlsRects[i].top, controlsRects[i].left + hudHeight, controlsRects[i].bottom },
//					stateelem.getPosition());
			drawString(mvp, ControlLabels[i], font, Color { (highlight ? difselcol : difcol).rgb(), displayAlpha * displaypercent },
					Vector2F { controlsRects[i].left + hudHeight * 1.5f, controlsRects[i].middle().y() }, hudHeight,
					Gravity::LEFT | Gravity::CENTER_VERTICAL);
		}
	}

}

void DemoController::setScene(SapphireScene* scene) {
}

#define BREAK_ON_NOT_DOWN()  if (KeyEvent::instance.getAction() != KeyAction::DOWN) break
bool DemoController::onKeyEvent() {
	auto kc = KeyEvent::instance.getKeycode();
	switch (kc) {
		case KeyCode::KEY_GAMEPAD_Y:
		case KeyCode::KEY_F1: {
			BREAK_ON_NOT_DOWN();
			if(controlsAnimation.get() == nullptr) {
				if(displayAlpha == 1.0f) {
					hideControls();
				} else {
					showControls();
				}
			}
			break;
		}
		case KeyCode::KEY_F2: {
			BREAK_ON_NOT_DOWN();
			setTurnSpeed(ControlIndex::FAST_BACK);
			break;
		}
		case KeyCode::KEY_F3: {
			BREAK_ON_NOT_DOWN();
			setTurnSpeed(ControlIndex::BACK);
			break;
		}
		case KeyCode::KEY_F4: {
			BREAK_ON_NOT_DOWN();
			setTurnSpeed(ControlIndex::SLOW_BACK);
			break;
		}
		case KeyCode::KEY_F5: {
			BREAK_ON_NOT_DOWN();
			setTurnSpeed(ControlIndex::PAUSE);
			break;
		}
		case KeyCode::KEY_F6: {
			BREAK_ON_NOT_DOWN();
			setTurnSpeed(ControlIndex::SLOW_PLAY);
			break;
		}
		case KeyCode::KEY_F7: {
			BREAK_ON_NOT_DOWN();
			setTurnSpeed(ControlIndex::PLAY);
			break;
		}
		case KeyCode::KEY_F8: {
			BREAK_ON_NOT_DOWN();
			setTurnSpeed(ControlIndex::FAST_PLAY);
			break;
		}
		case KeyCode::KEY_NUM_ADD:
		case KeyCode::KEY_PLUS:
		case KeyCode::KEY_GAMEPAD_RIGHT_SHOULDER: {
			BREAK_ON_NOT_DOWN();
			int selected = (int)getSelectedTurnSpeed() + 1;
			if(selected < CONTROLS_COUNT) {
				setTurnSpeed((ControlIndex)selected);
			}
			break;
		}
		case KeyCode::KEY_NUM_SUBTRACT:
		case KeyCode::KEY_MINUS:
		case KeyCode::KEY_GAMEPAD_LEFT_SHOULDER: {
			BREAK_ON_NOT_DOWN();
			int selected = (int)getSelectedTurnSpeed() - 1;
			if(selected >= 0) {
				setTurnSpeed((ControlIndex)selected);
			}
			break;
		}
		case KeyCode::KEY_GAMEPAD_DPAD_LEFT:
		case KeyCode::KEY_DIR_LEFT: {
			if(controlsAnimation.get() == nullptr && displayAlpha == 1.0f) {
				if(KeyEvent::instance.getAction() == KeyAction::DOWN) {
					int selected = (int)getSelectedTurnSpeed();
					if(selected > 0 && selected % controlsItemsPerRow > 0) {
						setTurnSpeed((ControlIndex)(selected - 1));
					}
				}
				return true;
			}
			return false;
		}
		case KeyCode::KEY_GAMEPAD_DPAD_RIGHT:
		case KeyCode::KEY_DIR_RIGHT: {
			if(controlsAnimation.get() == nullptr && displayAlpha == 1.0f) {
				if(KeyEvent::instance.getAction() == KeyAction::DOWN) {
					int selected = (int)getSelectedTurnSpeed();
					if(selected < CONTROLS_COUNT - 1 && selected % controlsItemsPerRow < controlsItemsPerRow - 1) {
						setTurnSpeed((ControlIndex)(selected + 1));
					}
				}
				return true;
			}
			return false;
		}
		case KeyCode::KEY_GAMEPAD_DPAD_UP:
		case KeyCode::KEY_DIR_UP: {
			if(controlsAnimation.get() == nullptr && displayAlpha == 1.0f) {
				if(KeyEvent::instance.getAction() == KeyAction::DOWN) {
					int selected = (int)getSelectedTurnSpeed();
					if(selected >= controlsItemsPerRow) {
						setTurnSpeed((ControlIndex)(selected - controlsItemsPerRow));
					}
				}
				return true;
			}
			return false;
		}
		case KeyCode::KEY_GAMEPAD_DPAD_DOWN:
		case KeyCode::KEY_DIR_DOWN: {
			if(controlsAnimation.get() == nullptr && displayAlpha == 1.0f) {
				if(KeyEvent::instance.getAction() == KeyAction::DOWN) {
					int selected = (int)getSelectedTurnSpeed();
					if(selected + controlsItemsPerRow < CONTROLS_COUNT) {
						setTurnSpeed((ControlIndex)(selected + controlsItemsPerRow));
					}
				}
				return true;
			}
			return false;
		}
		case KeyCode::KEY_GAMEPAD_A:
		case KeyCode::KEY_ENTER: {
			if(controlsAnimation.get() == nullptr && displayAlpha == 1.0f) {
				if(KeyEvent::instance.getAction() == KeyAction::DOWN) {
					hideControls();
				}
				return true;
			}
			return false;
		}
		default: {
			return false;
		}
	}
	return true;
}

bool DemoController::onTouchEvent() {
	auto* affected = TouchEvent::instance.getAffectedPointer();
	switch (TouchEvent::instance.getAction()) {
		case TouchAction::DOWN: {
			//here touched the screen, not any control or button
			auto touchpos = affected->getPosition();
			if (displayAlpha == 0.0f && controlButtonRect.isInside(touchpos)) {
				controlButtonTouch = affected;
			} else if (displayAlpha == 1.0f && !controlAreaRectangle.isInside(touchpos)) {
				controlsAnimation.kill();
				auto* anim = new ControlsAnimator(false, displayAlpha, core::MonotonicTime::getCurrent());
				controlsAnimation.link(*anim);
				anim->start();
			} else {
				for (unsigned int i = 0; i < CONTROLS_COUNT; ++i) {
					if (controlsRects[i].isInside(touchpos)) {
						controlsTouch[i] = affected;
						break;
					}
				}
			}
			break;
		}
		case TouchAction::MOVE_UPDATE: {
			if (controlButtonTouch != nullptr && !controlButtonRect.isInside(affected->getPosition())) {
				controlButtonTouch = nullptr;
			}
			for (unsigned int i = 0; i < CONTROLS_COUNT; ++i) {
				if (controlsTouch[i] != nullptr && !controlsRects[i].isInside(controlsTouch[i]->getPosition())) {
					controlsTouch[i] = nullptr;
				}
			}
			break;
		}
		case TouchAction::UP: {
			if (affected == controlButtonTouch) {
				controlButtonTouch = nullptr;
				showControls();

				controlsAnimation.kill();
				auto* anim = new ControlsAnimator(true, displayAlpha, core::MonotonicTime::getCurrent());
				controlsAnimation.link(*anim);
				anim->start();
			}
			for (unsigned int i = 0; i < CONTROLS_COUNT; ++i) {
				if (controlsTouch[i] == affected) {
					controlsTouch[i] = nullptr;
					setTurnSpeed((ControlIndex) i);
					break;
				}
			}
			break;
		}
		default: {
			break;
		}
	}
	return controlButtonTouch != nullptr || displayAlpha > 0.0f;
}

void DemoController::clearInput() {
	//TODO
	for (int i = 0; i < CONTROLS_COUNT; ++i) {
		controlsTouch[i] = nullptr;
	}
}

void DemoController::cancelTouch() {
	for (int i = 0; i < CONTROLS_COUNT; ++i) {
		controlsTouch[i] = nullptr;
	}
}

void DemoController::setTurnTime(long long ms) {
	if (ms != turnTime) {
		prevTurnTime = turnTime;
		turnTime = ms;
	}
}

void DemoController::setTurnSpeed(ControlIndex index) {
	ASSERT((unsigned int )index < CONTROLS_COUNT);
	playerControlIndex = index;
	switch (index) {
		case ControlIndex::FAST_BACK: {
			setTurnTime(-SAPPHIRE_TURN_FAST_MILLIS);
			playerAtlasIcon = VcrAtlas::CONTROL_FAST_BACK;
			break;
		}
		case ControlIndex::BACK: {
			setTurnTime(-SAPPHIRE_TURN_MILLIS);
			playerAtlasIcon = VcrAtlas::CONTROL_BACK;
			break;
		}
		case ControlIndex::SLOW_BACK: {
			setTurnTime(-SAPPHIRE_TURN_SLOW_MILLIS);
			playerAtlasIcon = VcrAtlas::CONTROL_SLOW_BACK;
			break;
		}
		case ControlIndex::PAUSE: {
			setTurnTime(0);
			playerAtlasIcon = useRecordingIcons ? VcrAtlas::CONTROL_RECORD_PAUSE : VcrAtlas::CONTROL_PAUSE;
			break;
		}
		case ControlIndex::SLOW_PLAY: {
			playerAtlasIcon = useRecordingIcons ? VcrAtlas::CONTROL_RECORD_SLOW_FORWARD : VcrAtlas::CONTROL_SLOW_FORWARD;
			setTurnTime(SAPPHIRE_TURN_SLOW_MILLIS);
			break;
		}
		case ControlIndex::PLAY: {
			playerAtlasIcon = useRecordingIcons ? VcrAtlas::CONTROL_RECORD_FORWARD : VcrAtlas::CONTROL_FORWARD;
			setTurnTime(SAPPHIRE_TURN_MILLIS);
			break;
		}
		case ControlIndex::FAST_PLAY: {
			playerAtlasIcon = useRecordingIcons ? VcrAtlas::CONTROL_RECORD_FAST_FORWARD : VcrAtlas::CONTROL_FAST_FORWARD;
			setTurnTime(SAPPHIRE_TURN_FAST_MILLIS);
			break;
		}
		default: {
			break;
		}
	}
}

void DemoController::setUseRecordingIcons() {
	ASSERT(!useRecordingIcons);

	this->useRecordingIcons = true;
	if (turnTime >= 0) {
		playerAtlasIcon = (VcrAtlas) ((int) playerAtlasIcon + ((int) VcrAtlas::CONTROL_RECORD_PAUSE - (int) VcrAtlas::CONTROL_PAUSE));

		controlElems[(unsigned int) ControlIndex::PAUSE] = &vcratlas->getAtIndex((unsigned int) VcrAtlas::CONTROL_RECORD_PAUSE);
		controlElems[(unsigned int) ControlIndex::PLAY] = &vcratlas->getAtIndex((unsigned int) VcrAtlas::CONTROL_RECORD_FORWARD);
		controlElems[(unsigned int) ControlIndex::SLOW_PLAY] = &vcratlas->getAtIndex((unsigned int) VcrAtlas::CONTROL_RECORD_SLOW_FORWARD);
		controlElems[(unsigned int) ControlIndex::FAST_PLAY] = &vcratlas->getAtIndex((unsigned int) VcrAtlas::CONTROL_RECORD_FAST_FORWARD);

		vcrIndexAtlas[(unsigned int) ControlIndex::PAUSE] = VcrAtlas::CONTROL_RECORD_PAUSE;
		vcrIndexAtlas[(unsigned int) ControlIndex::PLAY] = VcrAtlas::CONTROL_RECORD_FORWARD;
		vcrIndexAtlas[(unsigned int) ControlIndex::SLOW_PLAY] = VcrAtlas::CONTROL_RECORD_SLOW_FORWARD;
		vcrIndexAtlas[(unsigned int) ControlIndex::FAST_PLAY] = VcrAtlas::CONTROL_RECORD_FAST_FORWARD;
	}
}

void DemoController::showControls() {
	if (displayAlpha != 1.0f) {
		controlsAnimation.kill();
		auto* anim = new ControlsAnimator(true, displayAlpha, core::MonotonicTime::getCurrent());
		controlsAnimation.link(*anim);
		anim->start();
	}
}
void DemoController::hideControls() {
	if (displayAlpha != 0.0f) {
		controlsAnimation.kill();
		auto* anim = new ControlsAnimator(false, displayAlpha, core::MonotonicTime::getCurrent());
		controlsAnimation.link(*anim);
		anim->start();
	}
}

} // namespace userapp

