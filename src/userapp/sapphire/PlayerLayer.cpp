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
 * PlayerLayer.cpp
 *
 *  Created on: 2016. apr. 26.
 *      Author: sipka
 */

#include <framework/core/timing.h>
#include <framework/geometry/Matrix.h>
#include <framework/geometry/Vector.h>
#include <framework/io/key/KeyEvent.h>
#include <framework/io/touch/TouchEvent.h>
#include <framework/layer/Layer.h>
#include <framework/utils/LinkedList.h>
#include <framework/utils/utility.h>
#include <gen/assets.h>
#include <appmain.h>
#include <sapphire/dialogs/LevelFailedLayer.h>
#include <sapphire/dialogs/LevelSuccessLayer.h>
#include <sapphire/dialogs/PausedLayer.h>
#include <sapphire/PlayerLayer.h>
#include <sapphire/SapphireScene.h>
#include <sapphire/LevelSelectorLayer.h>
#include <sapphire/dialogs/items/EditTextDialogItem.h>
#include <sapphire/dialogs/ParentDismisserDialogLayer.h>
#include <sapphire/LevelEditorLayer.h>
#include <sapphire/dialogs/ShowDemoNameDialog.h>
#include <sapphire/sapphireconstants.h>
#include <sapphire/dialogs/GamePadStartDismissingDialog.h>

#include <sapphire/steam_opt.h>

#define _USE_MATH_DEFINES
#include <math.h>

#define RETURN_TEXT "Return to game"

namespace userapp {
using namespace rhfw;

static const unsigned int SPEED_LEVEL_COUNT = 5;

static const unsigned int SPEED_VALUES[SPEED_LEVEL_COUNT] = { SAPPHIRE_TURN_MILLIS, 125, 83, 62, 50 };
static const char* SPEED_LABELS[SPEED_LEVEL_COUNT] = { "Normal", "2x", "3x", "4x", "5x" };

PlayerLayer::PlayerLayer(SapphireUILayer* parent, const SapphireLevelDescriptor* desc)
		: SapphireUILayer(parent), descriptor(desc) {
	level.loadLevel(desc->getFileDescriptor());
	level.setRandomSeed((unsigned int) core::MonotonicTime::getCurrent());

	setNeedBackground(false);
	setColors(level.getInfo().difficulty);

	this->returnText = RETURN_TEXT;
}
PlayerLayer::PlayerLayer(SapphireUILayer* parent, const SapphireLevelDescriptor* desc, Level level)
		: SapphireUILayer(parent), level(util::move(level)), descriptor(desc) {

	setNeedBackground(false);
	setColors(level.getInfo().difficulty);

	this->returnText = RETURN_TEXT;
}
PlayerLayer::PlayerLayer(SapphireUILayer* parent, LevelEditorLayer* editorlayer)
		: SapphireUILayer(parent), level(editorlayer->getLevel()), descriptor(editorlayer->getDescriptor()), editorLayer(editorlayer) {
	level.resetState();

	setColors(level.getInfo().difficulty);

	setNeedBackground(false);
	this->level.setRandomSeed((unsigned int) core::MonotonicTime::getCurrent());

	this->returnText = RETURN_TEXT;
}
PlayerLayer::~PlayerLayer() {
	if (gamepadContext != nullptr) {
		gamepadContext->removeGamePadStateListener(this);
	}
}

void PlayerLayer::setLevel(const SapphireLevelDescriptor* desc) {
	auto* ss = static_cast<SapphireScene*>(getScene());
	commitPlayedTurns();

	if (descriptor != nullptr && !isTestingLevel()) {
		ss->notifyLevelPlayed(descriptor, level);
	}

	this->suspendedLevel = false;
	this->descriptor = desc;
	this->level.loadLevel(desc->getFileDescriptor());
	this->level.setRandomSeed((unsigned int) core::MonotonicTime::getCurrent());
	this->statisticsBase = level.getStatistics();
	setColors(level.getInfo().difficulty);
	if (desc != nullptr) {
		ss->updateLevelState(desc, LevelState::UNFINISHED);
	}

	auto& settings = ss->getSettings();
	drawer.setDrawer(settings.getArtStyleForLevel(level.getInfo().difficulty));

	sounder.levelReloaded();
	drawer.levelReloaded();

	controller.clearInput();

	if (selectorLayer != nullptr) {
		selectorLayer->displaySelection(desc);
	}
}

bool PlayerLayer::touchImpl() {
	if (!showing || !gainedInput || TouchEvent::instance.getAction() == TouchAction::CANCEL) {
		menuTouch = nullptr;
		controller.cancelTouch();
		return false;
	}
	auto* affected = TouchEvent::instance.getAffectedPointer();
	switch (TouchEvent::instance.getAction()) {
		case TouchAction::DOWN: {
			auto touchpos = affected->getPosition();

			if (menuTouch == nullptr && menuRect.isInside(touchpos)) {
				menuTouch = affected;
			} else {
				controller.onTouchEvent();
			}
			break;
		}
		case TouchAction::MOVE_UPDATE: {
			if (menuTouch != nullptr && !menuRect.isInside(menuTouch->getPosition())) {
				menuTouch = nullptr;
			} else {
				controller.onTouchEvent();
			}
			break;
		}
		case TouchAction::UP: {
			if (affected == menuTouch) {
				menuTouch = nullptr;
				showPausedDialog();
			} else {
				controller.onTouchEvent();
			}
			break;
		}
		default: {
			break;
		}
	}
	return true;
}

bool PlayerLayer::onKeyEventImpl() {
	if (!hasInputFocus()) {
		return false;
	}
	if (controller.onKeyEvent()) {
		return true;
	}
	KeyCode kc = KeyEvent::instance.getKeycode();
	switch (kc) {
		case KeyCode::KEY_GAMEPAD_START: {
			if (KeyEvent::instance.getAction() == KeyAction::DOWN) {
				showPausedDialog();
				return true;
			}
		}
			/* no break */
		default: {
			auto action = KeyEvent::instance.getAction();
			if (kc != KeyCode::KEY_UNKNOWN) {
				auto* ss = static_cast<SapphireScene*>(getScene());
				auto&& keymap = ss->getKeyMap();
				auto&& gpkeymap = ss->getGamePadKeyMap();
				if (action == KeyAction::UP) {
					if ((kc == keymap[SapphireKeyCode::KEY_RESET_LEVEL] || kc == gpkeymap[SapphireKeyCode::KEY_RESET_LEVEL])) {
						if (isSuspendedLevel()) {
							restartSuspendedLevel();
						} else {
							restartLevel();
						}
						return true;
					} else if ((kc == keymap[SapphireKeyCode::KEY_QUICK_SUSPEND] || kc == gpkeymap[SapphireKeyCode::KEY_QUICK_SUSPEND])) {
						if (getDescriptor() != nullptr && !isTestingLevel()) {
							quickSuspend();
							return true;
						}
					}
				} else if (action == KeyAction::DOWN) {
					if ((kc == keymap[SapphireKeyCode::KEY_INCREASE_SPEED] || kc == gpkeymap[SapphireKeyCode::KEY_INCREASE_SPEED])) {
						if (speedIndex + 1 < SPEED_LEVEL_COUNT) {
							speedIndex++;
						}
					} else if ((kc == keymap[SapphireKeyCode::KEY_DECREASE_SPEED] || kc == gpkeymap[SapphireKeyCode::KEY_DECREASE_SPEED])) {
						if (speedIndex > 0) {
							speedIndex--;
						}
					}
				}
			}
			break;
		}

	}
	return false;
}

void PlayerLayer::drawImpl(float displayPercent) {
	Color difcol = getUiColor();
	Color difselcol = getUiSelectedColor();
	difcol.a() = displayPercent;
	difselcol.a() = displayPercent;

	drawer.setPaddings(
			Rectangle { controller.getPaddings().left + menuRect.width(), controller.getPaddings().top + menuRect.height(),
					controller.getPaddings().right + menuRect.width(), controller.getPaddings().bottom + menuRect.height() });

	drawer.draw(turnPercent, displayPercent);
	hudDrawer.draw(level, displayPercent, &controller);

	renderer->setDepthTest(false);
	renderer->initDraw();

	controller.draw(displayPercent);

#ifdef SAPPHIRE_SCREENSHOT_MODE
	return;
#endif /* defined(SAPPHIRE_SCREENSHOT_MODE) */

	Matrix2D mvp;
	mvp.setScreenDimension(drawer.getSize().pixelSize);
	if (menuTouch != nullptr) {
		drawRectangleColor(mvp, difcol, menuRect);
	}
	drawSapphireTexture(mvp, menuTexture, menuTouch != nullptr ? difselcol : difcol, menuRect, Rectangle { 0, 0, 1, 1 });
	if (speedIndex != 0) {
		char buffer[256];
		snprintf(buffer, sizeof(buffer), "Speed: %s", SPEED_LABELS[speedIndex]);
		drawString(mvp, buffer, font, difcol,
				Vector2F { menuRect.height() * 0.25f, drawer.getSize().pixelSize.height() - menuRect.height() * 0.25f },
				menuRect.height() * 0.5f, Gravity::LEFT | Gravity::BOTTOM);
	}
}

void PlayerLayer::showPausedDialogChoose() {
	showPausedDialog();
}
void PlayerLayer::showPausedDialog() {
	auto* layer = new PausedLayer(this);
	layer->show(getScene(), true);
}

void PlayerLayer::sizeChanged(const core::WindowSize& size) {
	auto* ss = static_cast<SapphireScene*>(getScene());
	drawer.setSize(ss->getGameSize(), ss->getGameUserScale());
	hudDrawer.setSize(size, ss->getUiUserScale());

	float menuicondimcm = min(min(size.getPhysicalWidth() / 10.0f, size.getPhysicalHeight() / 8.0f), 1.5f);
	Size2F menuicondim = size.toPixels(Size2F { menuicondimcm, menuicondimcm });
	menuRect = Rectangle { size.pixelSize.width() - menuicondim.width(), 0, (float) size.pixelSize.width(), menuicondim.height() };

	controller.onSizeChanged(size);
}

void PlayerLayer::quickSuspend() {
	uncommittedStats += (level.getStatistics() - statisticsBase);
	statisticsBase = level.getStatistics();
	static_cast<SapphireScene*>(getScene())->suspendLevel(getDescriptor(), getLevel());
	this->suspendedLevel = true;
}
void PlayerLayer::restartLevel() {
	auto* ss = static_cast<SapphireScene*>(getScene());
	if (descriptor != nullptr && !isTestingLevel()) {
		ss->notifyLevelPlayed(descriptor, level);
	}
	if (isTestingLevel()) {
		level = editorLayer->getLevel();
		level.resetState();
	} else {
		uncommittedStats += (level.getStatistics() - statisticsBase);
		ASSERT(descriptor != nullptr);
		this->level.loadLevel(descriptor->getFileDescriptor());
		this->suspendedLevel = false;
	}
	this->turnPercent = 0.0f;
	level.setRandomSeed((unsigned int) core::MonotonicTime::getCurrent());
	drawer.levelReloaded();
	sounder.levelReloaded();

	controller.clearInput();

	statisticsBase = level.getStatistics();
}

void PlayerLayer::restartSuspendedLevel() {
	ASSERT(descriptor->hasSuspendedGame);
	ASSERT(descriptor != nullptr);

	auto* ss = static_cast<SapphireScene*>(getScene());
	if (!isTestingLevel()) {
		ss->notifyLevelPlayed(descriptor, level);
	}

	uncommittedStats += (level.getStatistics() - statisticsBase);

	this->level.loadLevel(descriptor->getFileDescriptor());
	this->turnPercent = 0.0f;
	controller.clearInput();

	suspendedLevel = true;

	ss->loadSuspendedLevel(descriptor, level);

	statisticsBase = level.getStatistics();
}

void PlayerLayer::onLosingInput() {
	SapphireUILayer::onLosingInput();
	controller.clearInput();
	steamOverlayCallback.Unregister();

}

void PlayerLayer::onGainingInput() {
	SapphireUILayer::onGainingInput();
	steamOverlayCallback.Register(this, &PlayerLayer::OnGameOverlayActivated);
}

void PlayerLayer::onHidingLayer() {
#if defined(SAPPHIRE_STEAM_API_AVAILABLE)
	auto* sf = SteamFriends();
	if (sf != nullptr) {
		sf->ClearRichPresence();
	}
#endif /* defined(SAPPHIRE_STEAM_API_AVAILABLE) */
}
void PlayerLayer::onShowingLayer() {
#if defined(SAPPHIRE_STEAM_API_AVAILABLE)
	if (!isTestingLevel()) {
		auto* sf = SteamFriends();
		if (sf != nullptr) {
			FixedString diff = isTestingLevel() ? "Testing" : difficultyToString(level.getInfo().difficulty);
			auto res = sf->SetRichPresence("status", diff + ": " + level.getInfo().title);
			ASSERT(res);
			if (descriptor != nullptr && !descriptor->isEditable()) {
				res = sf->SetRichPresence("connect", descriptor->uuid.asString());
				ASSERT(res);
			}
		}
	}
#endif /* defined(SAPPHIRE_STEAM_API_AVAILABLE) */
}

void PlayerLayer::setScene(Scene* scene) {
	SapphireUILayer::setScene(scene);
	auto* ss = static_cast<SapphireScene*>(scene);
	scene->getWindow()->foregroundTimeListeners += *this;
	controller.setScene(ss);
	if (descriptor != nullptr) {
		ss->updateLevelState(descriptor, LevelState::UNFINISHED);
	}
	sounder.attachToScene(ss);

	drawer.setDrawer(ss->getSettings().getArtStyleForLevel(level.getInfo().difficulty));

	hudDrawer.setScene(ss);

	ss->settingsChangedListeners += *this;

	if (gamepadContext != nullptr) {
		gamepadContext->addGamePadStateListener(this);
	}
}
void PlayerLayer::onSettingsChanged(const SapphireSettings& settings) {
	drawer.setDrawer(settings.getArtStyleForLevel(level.getInfo().difficulty));

	auto* ss = static_cast<SapphireScene*>(getScene());
	drawer.setSize(ss->getGameSize(), ss->getGameUserScale());
}

void PlayerLayer::onVisibilityToUserChanged(core::Window& window, bool visible) {
	if (!visible && hasInputFocus()) {
		showPausedDialogChoose();
	}
}

void PlayerLayer::showEndingDialog() {
	Color difcol = getUiColor();
	Color difselcol = getUiSelectedColor();
	if (isTestingLevel()) {
		ParentDismisserDialogLayer* layer = new ParentDismisserDialogLayer(this);
		if (level.isSuccessfullyOver()) {
			layer->setTitle("Level complete");
		} else {
			layer->setTitle("Level failed");
		}
		layer->addDialogItem(new CommandDialogItem { "Restart level", [this,layer] {
			this->restartLevel();
			layer->dismissKeepParent();
		} });
		layer->addDialogItem(new CommandDialogItem { "Return to editor", [this,layer] {
			layer->dismiss();
		} });
		if (level.isSuccessfullyOver()) {
			auto* savedemoitem = new CommandDialogItem { "Save as demo" };
			savedemoitem->setHandler([=] {
				ShowDemoNameDialog(layer, [=](FixedString name) {
							editorLayer->addDemo(new Demo(
											DemoInfo {util::move(name)},
											FixedString {level.getRecordedDemo(), level.getRecordedDemoLength()},
											level.getOriginalRandomSeed()
									));
							layer->removeDialogItem(savedemoitem);
							delete savedemoitem;
						});
			});
			layer->addDialogItem(savedemoitem);
		}
		layer->show(getScene(), true);

	} else {
		if (level.isSuccessfullyOver()) {
			SapphireScene* ss = static_cast<SapphireScene*>(getScene());
			const SapphireLevelDescriptor* next = selectorLayer == nullptr ? nullptr : ss->getNextLevel(descriptor);
			if (next != nullptr && next->difficulty != level.getInfo().difficulty) {
				SapphireScene::LevelPlayPermission perm;
				if (!ss->isAllowedToPlay(next->difficulty, &perm)) {
					next = ss->getFirstUnfinishedLevel(level.getInfo().difficulty, level.getPlayerCount());
				}
			}

			LevelSuccessLayer* layer = new LevelSuccessLayer(this, next);
			LOGI() << "Recorded demo: " << level.getOriginalRandomSeed() << " " << (const char*) FixedString { level.getRecordedDemo(),
					level.getRecordedDemoLength() };
			layer->showDialog(getScene());
		} else {
			DialogLayer* layer = new LevelFailedLayer(this);
			layer->showDialog(getScene());
		}
	}
}

bool PlayerLayer::onBackRequested() {
	showPausedDialog();
	return true;
}

void PlayerLayer::onTimeChanged(const core::time_millis& time, const core::time_millis& previous) {
	if (!hasInputFocus()) {
		return;
	}

	long long ms = (long long) (time - previous);
	advanceMilliseconds(ms);
}

void PlayerLayer::advanceMilliseconds(long long ms) {
	turnPercent += (float) ms / SPEED_VALUES[speedIndex];
	while (turnPercent >= 1.0f) {
		++playedTurns;
		turnPercent -= 1.0f;
		bool over = level.isOver();

		controller.applyControls();

		level.applyTurn();
		sounder.playSoundsForTurn();

		uncommittedStats += (level.getStatistics() - statisticsBase);
		statisticsBase = level.getStatistics();
		if (level.isOver()) {
			if (!over) {
				auto* ss = static_cast<SapphireScene*>(getScene());
				if (descriptor != nullptr) {
					if (!isTestingLevel()) {
						ss->notifyLevelPlayed(descriptor, level);
					}
					if (level.isSuccessfullyOver()) {
						ss->updateLevelState(descriptor, LevelState::COMPLETED, level.getRecordedDemoString(),
								level.getOriginalRandomSeed(), &level.getStatistics());
					}
				}
				overTurn = level.getTurn();
				finishStatistics = level.getStatistics();
			} else if (overTurn > 0 && level.getTurn() - overTurn == 6) {
				showEndingDialog();
				commitPlayedTurns();
			}
		}
	}
}

void PlayerLayer::onInputFocusChanged(core::Window& window, bool inputFocused) {
	if (!inputFocused && hasInputFocus()) {
		showPausedDialogChoose();
	}
}

void PlayerLayer::dismiss() {
	SapphireUILayer::dismiss();
	commitPlayedTurns();
}
void PlayerLayer::commitPlayedTurns() {
	if (descriptor != nullptr) {
		auto* ss = static_cast<SapphireScene*>(getScene());
		ss->commitTimePlayedOnLevel(descriptor, playedTurns);
		LOGTRACE() << "Commit statistics turns: " << uncommittedStats.turns;
		ss->commitStatisticsPlayedOnLevel(descriptor, uncommittedStats);
		uncommittedStats = LevelStatistics { };
		this->playedTurns = 0;
	}
}

void PlayerLayer::onGamePadAttached(GamePad* gamepad) {
}
void PlayerLayer::onGamePadDetached(GamePad* gamepad) {
	if (hasInputFocus()) {
		showPausedDialog();
	}
}

#if defined(SAPPHIRE_STEAM_API_AVAILABLE)

void PlayerLayer::OnGameOverlayActivated(GameOverlayActivated_t* pParam) {
	if (pParam->m_bActive) {
		showPausedDialog();
	}
}

#endif /* defined(SAPPHIRE_STEAM_API_AVAILABLE) */

} // namespace userapp

