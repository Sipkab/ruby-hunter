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
 * LevelController.cpp
 *
 *  Created on: 2016. dec. 17.
 *      Author: sipka
 */

#include <sapphire/LevelController.h>
#include <sapphire/SapphireScene.h>
#include <framework/utils/utility.h>
#include <framework/geometry/Vector.h>
#include <framework/io/key/KeyEvent.h>
#include <appmain.h>

#define _USE_MATH_DEFINES
#include <math.h>

namespace userapp {

template<>
void LevelController::PlayerControlTracker::controlDown<SapphireControl::PutBomb>() {
	hadBomb = !hadBomb;
}
template<>
void LevelController::PlayerControlTracker::controlUp<SapphireControl::PutBomb>() {
}

template<>
void LevelController::PlayerControlTracker::controlDown<SapphireControl::Take>() {
}
template<>
void LevelController::PlayerControlTracker::controlUp<SapphireControl::Take>() {
}

LevelController::LevelController(Level* level)
		: level(level) {
#ifdef SAPPHIRE_NO_ONSCREEN_CONTROLS
	controlsVisibility = 0.0f;
#endif /* defined(SAPPHIRE_NO_ONSCREEN_CONTROLS) */

}
LevelController::~LevelController() {
}

bool LevelController::onKeyEvent() {
	if (KeyEvent::instance.getAction() == KeyAction::DOWN || KeyEvent::instance.getAction() == KeyAction::UP) {
		switch (KeyEvent::instance.getInputDevice()) {
			case InputDevice::KEYBOARD: {
				auto aptr = keyboardActions[KeyEvent::instance.getKeycode()];
				if (aptr != nullptr) {
					return aptr(this, KeyEvent::instance.getAction() == KeyAction::DOWN, -1);
				}
				break;
			}
			case InputDevice::GAMEPAD: {
				int playerid = scene->getPlayerIdForGamePad(KeyEvent::instance.getGamePad());
				if (playerid < 0) {
					return false;
				}
				auto aptr = gamepadActions[playerid][KeyEvent::instance.getKeycode()];
				if (aptr != nullptr) {
					return aptr(this, KeyEvent::instance.getAction() == KeyAction::DOWN, playerid);
				}
				break;
			}
			default: {
				break;
			}
		}
	}
	return false;
}

bool LevelController::onTouchEvent() {
#ifdef SAPPHIRE_NO_ONSCREEN_CONTROLS
	return false;
#endif /* defined(SAPPHIRE_NO_ONSCREEN_CONTROLS) */

	auto* affected = TouchEvent::instance.getAffectedPointer();
	switch (TouchEvent::instance.getAction()) {
		case TouchAction::DOWN: {
			auto touchpos = affected->getPosition();

			if (controlTrackers[0].placeBombTouch == nullptr && placeBombTouchRect.isInside(touchpos)) {
				controlTrackers[0].controlDown<SapphireControl::PutBomb>();
				controlTrackers[0].placeBombTouch = affected;
			} else if (controlTrackers[0].pickTouch == nullptr && pickAxeTouchRect.isInside(touchpos)) {
				controlTrackers[0].controlDown<SapphireControl::Take>();
				controlTrackers[0].pickTouch = affected;
			} else if (directionTouch == nullptr && (touchpos - directionMiddle).length() <= directionRadius) {
				directionTouch = affected;
				touchDirectionDown = getTouchedDirection(touchpos);
				controlTrackers[0].dirDown(touchDirectionDown);
			}
			break;
		}
		case TouchAction::MOVE_UPDATE: {
			if (directionTouch != nullptr) {
				//do not test out of bounds
				SapphireDirection ndir = getTouchedDirection(directionTouch->getPosition());
				if (ndir != touchDirectionDown) {
					if (touchDirectionDown == SapphireDirection::Undefined
							|| (directionTouch->getPosition() - directionMiddle).length() >= directionRadius / 5.5f) {
						controlTrackers[0].dirUp(touchDirectionDown);
						touchDirectionDown = ndir;
						controlTrackers[0].dirDown(touchDirectionDown);
					}
				}
			} else {
				findNewDirectionTouchPointer();
			}
			if (controlTrackers[0].pickTouch != nullptr && !pickAxeTouchRect.isInside(controlTrackers[0].pickTouch->getPosition())) {
				controlTrackers[0].pickTouch = nullptr;
				controlTrackers[0].controlUp<SapphireControl::Take>();
			}
			if (controlTrackers[0].placeBombTouch != nullptr
					&& !placeBombTouchRect.isInside(controlTrackers[0].placeBombTouch->getPosition())) {
				controlTrackers[0].placeBombTouch = nullptr;
				controlTrackers[0].controlUp<SapphireControl::PutBomb>();
			}
			break;
		}
		case TouchAction::UP: {
			if (affected == controlTrackers[0].pickTouch) {
				controlTrackers[0].pickTouch = nullptr;
				controlTrackers[0].controlUp<SapphireControl::Take>();
			} else if (affected == controlTrackers[0].placeBombTouch) {
				controlTrackers[0].placeBombTouch = nullptr;
				controlTrackers[0].controlUp<SapphireControl::PutBomb>();
			} else if (affected == directionTouch) {
				directionTouch = nullptr;
				controlTrackers[0].dirUp(touchDirectionDown);
				touchDirectionDown = SapphireDirection::Undefined;
				findNewDirectionTouchPointer();
			}
			break;
		}
		case TouchAction::CANCEL: {
			cancelTouch();
			break;
		}
		default: {
			break;
		}
	}
	return controlTrackers[0].isAnyTouchDown();
}
void LevelController::cancelTouch() {
	directionTouch = nullptr;
	touchDirectionDown = SapphireDirection::Undefined;
	for (int i = 0; i < 2; ++i) {
		for (int j = 0; j < 4; j++) {
			controlTrackers[i].dirUp((SapphireDirection) j);
		}
		controlTrackers[i].pickTouch = nullptr;
		controlTrackers[i].placeBombTouch = nullptr;
	}
}

void LevelController::setScene(SapphireScene* scene) {
	applyKeyMap(scene->getKeyMap());
	applyGamePadKeyMap(scene->getGamePadKeyMap());

	scene->keyMapChangedListeners += *this;
	scene->settingsChangedListeners += *this;

	this->scene = scene;
}
void LevelController::onKeyMapChanged(const SapphireKeyMap& keymap, const SapphireKeyMap& gamepadkeymap) {
	//TODO more efficent nulling out
	keyboardActions.clear();
	for (auto&& gpaction : gamepadActions) {
		gpaction.clear();
	}
	applyKeyMap(keymap);
	applyGamePadKeyMap(gamepadkeymap);
}
class LevelControllerExecutor {
public:
	template<SapphireDirection Dir, unsigned int PlayerId>
	static bool dirKey(LevelController* lc, bool down, int unusedplayerid) {
		lc->animateHadKeyboard();
		lc->controlTrackers[PlayerId].keyDirectionAction(Dir, down);

		return PlayerId < lc->level->getPlayerCount();
	}

	template<unsigned int PlayerId>
	static bool bombKey(LevelController* lc, bool down, int unusedplayerid) {
		lc->animateHadKeyboard();
		lc->controlTrackers[PlayerId].bombKeyEvent(down);

		return PlayerId < lc->level->getPlayerCount();
	}
	template<unsigned int PlayerId>
	static bool pickKey(LevelController* lc, bool down, int unusedplayerid) {
		lc->animateHadKeyboard();
		lc->controlTrackers[PlayerId].pickKeyEvent(down);

		return PlayerId < lc->level->getPlayerCount();
	}

	static bool pickKeyPlayerId(LevelController* lc, bool down, int playerid) {
		ASSERT(playerid >= 0 && playerid < SAPPHIRE_MAX_PLAYER_COUNT) << playerid;
		lc->animateHadKeyboard();
		lc->controlTrackers[playerid].pickKeyEvent(down);

		return playerid < lc->level->getPlayerCount();
	}

	template<SapphireDirection Dir>
	static bool dirKeyPlayerId(LevelController* lc, bool down, int playerid) {
		ASSERT(playerid >= 0 && playerid < SAPPHIRE_MAX_PLAYER_COUNT) << playerid;
		lc->animateHadKeyboard();
		lc->controlTrackers[playerid].keyDirectionAction(Dir, down);

		return playerid < lc->level->getPlayerCount();
	}

	static bool bombKeyPlayerId(LevelController* lc, bool down, int playerid) {
		ASSERT(playerid >= 0 && playerid < SAPPHIRE_MAX_PLAYER_COUNT) << playerid;
		lc->animateHadKeyboard();
		lc->controlTrackers[playerid].bombKeyEvent(down);

		return playerid < lc->level->getPlayerCount();
	}

};
void LevelController::ControllerKeyActions::setKeyAction(KeyCode kc, KeyActionFunction func) {
	if (kc == KeyCode::KEY_SHIFT) {
		setKeyAction(KeyCode::KEY_LEFT_SHIFT, func);
		setKeyAction(KeyCode::KEY_RIGHT_SHIFT, func);
	} else if (kc == KeyCode::KEY_CTRL) {
		setKeyAction(KeyCode::KEY_LEFT_CTRL, func);
		setKeyAction(KeyCode::KEY_RIGHT_CTRL, func);
	} else if (kc == KeyCode::KEY_ALT) {
		setKeyAction(KeyCode::KEY_LEFT_ALT, func);
		setKeyAction(KeyCode::KEY_RIGHT_ALT, func);
	}
	keyActions[(unsigned int) kc] = func;
}
void LevelController::applyKeyMap(const SapphireKeyMap& map) {
	keyboardActions.setKeyAction(map[SapphireKeyCode::KEY_P1_UP], LevelControllerExecutor::dirKey<SapphireDirection::Up, 0>);
	keyboardActions.setKeyAction(map[SapphireKeyCode::KEY_P1_LEFT], LevelControllerExecutor::dirKey<SapphireDirection::Left, 0>);
	keyboardActions.setKeyAction(map[SapphireKeyCode::KEY_P1_DOWN], LevelControllerExecutor::dirKey<SapphireDirection::Down, 0>);
	keyboardActions.setKeyAction(map[SapphireKeyCode::KEY_P1_RIGHT], LevelControllerExecutor::dirKey<SapphireDirection::Right, 0>);
	keyboardActions.setKeyAction(map[SapphireKeyCode::KEY_P1_PICK], LevelControllerExecutor::pickKey<0>);
	keyboardActions.setKeyAction(map[SapphireKeyCode::KEY_P1_BOMB], LevelControllerExecutor::bombKey<0>);

#ifdef SAPPHIRE_DUAL_PLAYER_AVAILABLE
	keyboardActions.setKeyAction(map[SapphireKeyCode::KEY_P2_UP], LevelControllerExecutor::dirKey<SapphireDirection::Up, 1>);
	keyboardActions.setKeyAction(map[SapphireKeyCode::KEY_P2_LEFT], LevelControllerExecutor::dirKey<SapphireDirection::Left, 1>);
	keyboardActions.setKeyAction(map[SapphireKeyCode::KEY_P2_DOWN], LevelControllerExecutor::dirKey<SapphireDirection::Down, 1>);
	keyboardActions.setKeyAction(map[SapphireKeyCode::KEY_P2_RIGHT], LevelControllerExecutor::dirKey<SapphireDirection::Right, 1>);
	keyboardActions.setKeyAction(map[SapphireKeyCode::KEY_P2_PICK], LevelControllerExecutor::pickKey<1>);
	keyboardActions.setKeyAction(map[SapphireKeyCode::KEY_P2_BOMB], LevelControllerExecutor::bombKey<1>);
#endif /* defined(SAPPHIRE_DUAL_PLAYER_AVAILABLE) */
}
void LevelController::applyGamePadKeyMap(const SapphireKeyMap& map) {
	gamepadActions[0].setKeyAction(map[SapphireKeyCode::KEY_P1_UP], LevelControllerExecutor::dirKey<SapphireDirection::Up, 0>);
	gamepadActions[0].setKeyAction(map[SapphireKeyCode::KEY_P1_LEFT], LevelControllerExecutor::dirKey<SapphireDirection::Left, 0>);
	gamepadActions[0].setKeyAction(map[SapphireKeyCode::KEY_P1_DOWN], LevelControllerExecutor::dirKey<SapphireDirection::Down, 0>);
	gamepadActions[0].setKeyAction(map[SapphireKeyCode::KEY_P1_RIGHT], LevelControllerExecutor::dirKey<SapphireDirection::Right, 0>);
	gamepadActions[0].setKeyAction(map[SapphireKeyCode::KEY_P1_PICK], LevelControllerExecutor::pickKey<0>);
	gamepadActions[0].setKeyAction(map[SapphireKeyCode::KEY_P1_BOMB], LevelControllerExecutor::bombKey<0>);

#ifdef SAPPHIRE_DUAL_PLAYER_AVAILABLE
	gamepadActions[1].setKeyAction(map[SapphireKeyCode::KEY_P2_UP], LevelControllerExecutor::dirKey<SapphireDirection::Up, 1>);
	gamepadActions[1].setKeyAction(map[SapphireKeyCode::KEY_P2_LEFT], LevelControllerExecutor::dirKey<SapphireDirection::Left, 1>);
	gamepadActions[1].setKeyAction(map[SapphireKeyCode::KEY_P2_DOWN], LevelControllerExecutor::dirKey<SapphireDirection::Down, 1>);
	gamepadActions[1].setKeyAction(map[SapphireKeyCode::KEY_P2_RIGHT], LevelControllerExecutor::dirKey<SapphireDirection::Right, 1>);
	gamepadActions[1].setKeyAction(map[SapphireKeyCode::KEY_P2_PICK], LevelControllerExecutor::pickKey<1>);
	gamepadActions[1].setKeyAction(map[SapphireKeyCode::KEY_P2_BOMB], LevelControllerExecutor::bombKey<1>);
#endif /* defined(SAPPHIRE_DUAL_PLAYER_AVAILABLE) */
}
void LevelController::onSizeChanged(const core::WindowSize& size) {
	mvp.setScreenDimension(size.pixelSize.width(), size.pixelSize.height());

	updateControlPositions(scene->getUiSize(), scene->getSettings());

	//float sidepadding;
	auto* anim = controlAnimator.get();
	if (anim != nullptr) {
		controlAnimator.kill();
		controlsVisibility = 0.0f;
		//sidepadding = menuicondim.width();
	} else {
		if (controlsVisibility > 0.0f) {
			//sidepadding = max(menuicondim.width(), directionssize);
		} else {
			//sidepadding = menuicondim.width();
		}
	}

	if (controlsVisibility > 0.0f) {
		paddings.left = paddings.right = max(pickAxeTouchRect.width(), directionRadius * 2);
	} else {
		paddings.left = paddings.right = 0.0f;
	}
}

void LevelController::draw(float displaypercent) {
#ifdef SAPPHIRE_NO_ONSCREEN_CONTROLS
	return;
#endif /* defined(SAPPHIRE_NO_ONSCREEN_CONTROLS) */

	Color difcol = difficultyToColor(level->getInfo().difficulty);
	Color difselcol = difficultyToSelectedColor(level->getInfo().difficulty);

	renderer->setDepthTest(false);
	renderer->initDraw();

	float controlsalpha = displaypercent * controlsVisibility;

	if (controlsalpha > 0.0f) {
		Vector2F diagdir = { 1, 1 };
		diagdir.normalize();
		diagdir *= directionRadius;

		drawRectangleColor(Matrix2D().setRotate(M_PI_4).multTranslate(directionMiddle) *= mvp, Color { difcol.xyz(), controlsalpha },
				Rectangle { -diagdir.x(), -directionsSeparatorOffset.y(), diagdir.x(), directionsSeparatorOffset.y() });
		drawRectangleColor(Matrix2D().setRotate(-M_PI_4).multTranslate(directionMiddle) *= mvp, Color { difcol.xyz(), controlsalpha },
				Rectangle { -diagdir.x(), -directionsSeparatorOffset.y(), diagdir.x(), directionsSeparatorOffset.y() });

		for (int i = 0; i < 4; ++i) {
			bool down = controlTrackers[0].isDirDown((SapphireDirection) i);
			if (down) {
				drawRectangleColor(
						Matrix2D().setRotate(M_PI_4).multTranslate(-directionRadius / 2, 0).multRotate(M_PI / -2 * i).multTranslate(
								directionMiddle) *= mvp, Color { difcol.xyz(), controlsalpha }, Rectangle { -diagdir / 2, diagdir / 2 });
			}
			drawSapphireTexture(Matrix2D().setRotate(-M_PI_2 * i).multTranslate(directionMiddle) *= mvp, arrowTexture,
					Color { (down ? difselcol : difcol).xyz(), controlsalpha },
					Rectangle { -diagdir.x(), -diagdir.y() / 2, -diagdir.x() / 2, diagdir.y() / 2 }, Rectangle { 0, 0, 1, 1 });
		}

		drawSapphireTexture(mvp, circleTexture,
				controlTrackers[0].pickKeyDown || controlTrackers[0].pickTouch != nullptr ? Color { difcol.xyz(), controlsalpha } : Color {
																									1, 1, 1, controlsalpha * 0.2f },
				pickAxeDrawRect, Rectangle { 0, 0, 1, 1 });
		drawSapphireTexture(mvp, circleTexture,
				controlTrackers[0].bombKeyDown || controlTrackers[0].placeBombTouch != nullptr ?
						Color { difcol.xyz(), controlsalpha } : Color { 1, 1, 1, controlsalpha * 0.2f }, placeBombDrawRect, Rectangle { 0,
						0, 1, 1 });

		drawSapphireTexture(mvp, pickaxeTexture, controlsalpha, Rectangle { pickAxeDrawRect.inset(placeBombDrawRect.widthHeight() / 6.0f) },
				Rectangle { 0, 0, 1, 1 });
		auto& tickbombelem = tickbombAnimation->getAtIndex(0);
		drawSapphireTexture(mvp, tickbombelem, controlsalpha, Rectangle { placeBombDrawRect.inset(placeBombDrawRect.widthHeight() / 6.0f) },
				tickbombelem.getPosition());
	}
}

void LevelController::PlayerControlTracker::bombKeyEvent(bool down) {
	if (down) {
		if (!bombKeyDown) {
			controlDown<SapphireControl::PutBomb>();
			bombKeyDown = true;
		}
	} else {
		controlUp<SapphireControl::PutBomb>();
		bombKeyDown = false;
	}
}

void LevelController::PlayerControlTracker::pickKeyEvent(bool down) {
	if (down) {
		if (!pickKeyDown) {
			controlDown<SapphireControl::Take>();
			pickKeyDown = true;
		}
	} else {
		controlUp<SapphireControl::Take>();
		pickKeyDown = false;
	}
}
void LevelController::PlayerControlTracker::keyDirectionAction(SapphireDirection dir, bool down) {
	if (down) {
		if (!keysDirectionsDown[(unsigned int) dir]) {
			keyDown(dir);
		}
	} else {
		keyUp(dir);
	}
}
void LevelController::PlayerControlTracker::keyDirectionAction(SapphireDirection dir, KeyAction action) {
	switch (action) {
		case KeyAction::DOWN: {
			keyDirectionAction(dir, true);
			break;
		}
		case KeyAction::UP: {
			keyDirectionAction(dir, false);
			break;
		}
		default: {
			break;
		}
	}
}

void LevelController::animateHadKeyboard() {
	if (!hadKeyboard) {
		hadKeyboard = true;
		auto* anim = new ControlsAnimator(paddings, controlsVisibility, 0.0f, core::MonotonicTime::getCurrent(), core::time_millis { 200 });
		anim->start();
		controlAnimator.link(anim);
	}
}

void LevelController::applyControls() {
	unsigned int pc = level->getPlayerCount();
	for (unsigned int i = 0; i < pc; ++i) {
		if (!controlTrackers[i].commands.isEmpty()) {
			auto* first = static_cast<ControlCommand*>(*controlTrackers[i].commands.nodes().begin());
			level->applyControl(i, first->dir, first->control);
			delete first;
		} else {
			auto dir = controlTrackers[i].getDirection();
			if (dir != SapphireDirection::Undefined) {
				level->applyControl(i, dir, controlTrackers[i].getControl());
			}
		}
	}
}

void LevelController::findNewDirectionTouchPointer() {
	if (directionTouch != nullptr) {
		return;
	}
	unsigned int count = TouchEvent::instance.getPointerCount();
	unsigned int lastid = 0;
	for (unsigned int i = 0; i < count; i++) {
		auto* ptr = TouchEvent::instance.findNextPointer(lastid);
		lastid = ptr->getId();
		if (ptr != controlTrackers[0].pickTouch && ptr != controlTrackers[0].placeBombTouch
				&& (ptr->getPosition() - directionMiddle).length() <= directionRadius) {
			directionTouch = ptr;
			touchDirectionDown = getTouchedDirection(ptr->getPosition());
			controlTrackers[0].dirDown(touchDirectionDown);
			return;
		}
	}
}
SapphireDirection LevelController::getTouchedDirection(Vector2F pos) {
	Vector2F diagdir { 1, 1 };
	Vector2F diagnormaldir { -1, 1 };
	pos = pos - directionMiddle;

	float dotdiag = diagdir.dot(pos);
	float dotdiagnorm = diagnormaldir.dot(pos);

	if (dotdiag > 0.0f) {
		if (dotdiagnorm > 0.0f) {
			return SapphireDirection::Down;
		}
		return SapphireDirection::Right;
	}
	if (dotdiagnorm > 0.0f) {
		return SapphireDirection::Left;
	}
	return SapphireDirection::Up;
}

void LevelController::clearInput() {
	for (int i = 0; i < 2; ++i) {
		for (int j = 0; j < 4; j++) {
			controlTrackers[i].touchDirectionsDown[j] = false;
			controlTrackers[i].keysDirectionsDown[j] = false;
		}
		controlTrackers[i].commands.clear();
		controlTrackers[i].bombKeyDown = false;
		controlTrackers[i].pickKeyDown = false;
		controlTrackers[i].hadBomb = false;
	}
}

bool LevelController::hasActions() {
	unsigned int pc = level->getPlayerCount();
	for (unsigned int i = 0; i < pc; ++i) {
		if (!controlTrackers[i].commands.isEmpty() || controlTrackers[i].getDirection() != SapphireDirection::Undefined) {
			return true;
		}
	}
	return false;
}
bool LevelController::hasAllActions() {
	return getActedPlayerCount() == level->getPlayerCount();
}

unsigned int LevelController::getActedPlayerCount() const {
	unsigned int pc = level->getPlayerCount();
	unsigned int c = 0;
	for (unsigned int i = 0; i < pc; ++i) {
		if (!controlTrackers[i].commands.isEmpty() || controlTrackers[i].getDirection() != SapphireDirection::Undefined) {
			++c;
		}
	}
	return c;
}

void LevelController::onSettingsChanged(const SapphireSettings& settings) {
	updateControlPositions(scene->getUiSize(), settings);
}

void LevelController::updateControlPositions(const core::WindowSize& size, const SapphireSettings& settings) {
	float scale = scene->getUiUserScale();
	float hudHeight = size.toPixelsY(min(size.getPhysicalSize().height() / (18 / scale), 1.0f * scale));

	float directionssize = min(size.toPixelsY(4.0f), size.pixelSize.height() * 2.0f / 3.0f);
	directionRadius = directionssize / 2.0f;

	directionMiddle = Vector2F { directionssize / 2.0f, size.pixelSize.height() - directionssize / 2.0f };
	directionsSeparatorOffset = size.toPixels(Size2F { -0.025f, 0.025f });

	directionMiddle.y() -= hudHeight;
	pickAxeTouchRect = Rectangle { size.pixelSize - Size2F { directionssize / 2.0f, directionssize / 2.0f }, size.pixelSize }.translate(
			Vector2F { 0, -hudHeight });

	if (settings.leftHandedOnScreenControls) {
		directionMiddle.x() = size.pixelSize.x() - directionMiddle.x();
		pickAxeTouchRect.right = pickAxeTouchRect.width();
		pickAxeTouchRect.left = 0;
	}
	placeBombTouchRect = pickAxeTouchRect.translate(Vector2F { 0, -pickAxeTouchRect.height() });

	pickAxeDrawRect = pickAxeTouchRect.inset(pickAxeTouchRect.widthHeight() / 8.0f);
	placeBombDrawRect = placeBombTouchRect.inset(placeBombTouchRect.widthHeight() / 8.0f);
}

void LevelController::hideControls() {
	hadKeyboard = true;
	controlsVisibility = 0.0f;
	controlAnimator.kill();
	paddings.left = paddings.right = 0.0f;
}

} // namespace userapp

