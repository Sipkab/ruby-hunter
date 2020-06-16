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
 * DemoRecorderLayer.cpp
 *
 *  Created on: 2016. dec. 17.
 *      Author: sipka
 */

#include <sapphire/DemoRecorderLayer.h>
#include <sapphire/dialogs/DialogLayer.h>
#include <sapphire/dialogs/SettingsLayer.h>
#include <sapphire/LevelEditorLayer.h>
#include <sapphire/dialogs/ShowDemoNameDialog.h>
#include <sapphire/SapphireScene.h>
#include <sapphire/dialogs/GamePadStartDismissingDialog.h>
#include <sapphire/level/Demo.h>
#include <framework/io/key/KeyEvent.h>

namespace userapp {

DemoRecorderLayer::DemoRecorderLayer(SapphireUILayer* parent, const SapphireLevelDescriptor* descriptor, Level level)
		: DemoLayer(parent, descriptor, util::move(level)), levelController(&this->level) {
	demoController.setDemoSteps(this->demoSteps, 0);
	demoController.setUseRecordingIcons();
	setRandomSeed((unsigned int) core::MonotonicTime::getCurrent());

	this->returnText = "Return to demo recorder";
}

void DemoRecorderLayer::onPausedIdle(long long ms) {
	unsigned int acted = levelController.getActedPlayerCount();
	if (acted == level.getPlayerCount()) {
		advanceNextTurnPaused();
	} else if (acted == 0) {
		actStart = 0;
		if (spaceDown || spaceTouch != nullptr) {
			advanceNextTurnPaused();
		} else {
			if (!endResumed && isOver()) {
				advanceNextTurnPaused();
			}
		}
	} else {
		actStart += ms;
		if (actStart >= 50) {
			advanceNextTurnPaused();
		}
	}
}
int DemoRecorderLayer::getOverTurns() {
	if (!isOver()) {
		return -1;
	}
	return overTurn;
}
bool DemoRecorderLayer::isOver() {
	return level.isSuccessfullyOver();
}

void DemoRecorderLayer::promptSaveDemoAndExit(SapphireUILayer* dialog) {
	if (editorLayer != nullptr) {
		ShowDemoNameDialog(dialog, [=]( FixedString name) {
			editorLayer->addDemo(new Demo(
							DemoInfo {util::move(name)},
							FixedString {level.getRecordedDemo(), level.getRecordedDemoLength()},
							level.getOriginalRandomSeed()
					));
			dialog->dismiss();
			this->dismiss();
		});
	} else {
		ASSERT(descriptor != nullptr);
		ShowDemoNameDialog(dialog, [=](const FixedString& name) {
			static_cast<SapphireScene*>(getScene())->saveDemo(
					descriptor,
					name,
					level.getOriginalRandomSeed(),
					level.getRecordedDemo(),
					level.getRecordedDemoLength());
			dialog->dismiss();
			this->dismiss();
		});
	}
}

void DemoRecorderLayer::onLosingInput() {
	DemoLayer::onLosingInput();
	spaceDown = false;
	spaceTouch = nullptr;
	demoController.clearInput();
	levelController.clearInput();
}

void DemoRecorderLayer::drawDemoRelated(float displaypercent) {
	levelController.draw(displaypercent);
}

DemoRecorderLayer::~DemoRecorderLayer() {
	if (gamepadContext != nullptr) {
		gamepadContext->removeGamePadStateListener(this);
	}

	delete[] demoSteps;
}

bool DemoRecorderLayer::touchImpl() {
	if (spaceTouch != nullptr && !spaceRect.isInside(spaceTouch->getPosition())) {
		spaceTouch = nullptr;
	}
	if (DemoLayer::touchImpl()) {
		return true;
	}
	if (demoController.getTurnTime() >= 0) {
		if (levelController.onTouchEvent()) {
			return true;
		}
		switch (TouchEvent::instance.getAction()) {
			case TouchAction::DOWN: {
				auto* affected = TouchEvent::instance.getAffectedPointer();
				if (spaceRect.isInside(affected->getPosition())) {
					spaceTouch = affected;
				}
				break;
			}
			case TouchAction::UP: {
				if (spaceTouch != nullptr && !spaceTouch->isDown()) {
					spaceTouch = nullptr;
				}
				break;
			}
			default: {
				break;
			}
		}
	}
	return true;
}
bool DemoRecorderLayer::onKeyEventImpl() {
	if (DemoLayer::onKeyEventImpl()) {
		return true;
	}
	if (demoController.getTurnTime() >= 0 && levelController.onKeyEvent()) {
		return true;
	}
	switch (KeyEvent::instance.getKeycode()) {
		case KeyCode::KEY_GAMEPAD_X:
		case KeyCode::KEY_SPACE: {
			switch (KeyEvent::instance.getAction()) {
				case KeyAction::DOWN: {
					spaceDown = true;
					break;
				}
				case KeyAction::UP: {
					spaceDown = false;
					break;
				}
				default: {
					break;
				}
			}
			return true;
		}
		default: {
			break;
		}
	}
	return false;
}

void DemoRecorderLayer::goToNextTurn() {
	levelController.applyControls();
	level.applyTurn();

	if (level.getRecordedDemoLength() > capacity) {
		ASSERT(level.getRecordedDemoLength() <= capacity * 2);
		capacity *= 2;
		char* nsteps = new char[capacity];
		memcpy(nsteps, level.getRecordedDemo(), level.getRecordedDemoLength());

		delete[] demoSteps;
		demoSteps = nsteps;
	} else {
		unsigned int plrcount = level.getPlayerCount();
		for (int i = 0; i < plrcount; ++i) {
			demoSteps[level.getRecordedDemoLength() - 1 - i] = level.getRecordedDemo()[level.getRecordedDemoLength() - 1 - i];
		}
	}
	demoController.setDemoSteps(this->demoSteps, this->level.getRecordedDemoLength());
	if (overTurn == 0 && isOver()) {
		overTurn = level.getTurn();
	}
}

void DemoRecorderLayer::goToPreviousTurn(unsigned int target) {
	DemoLayer::goToPreviousTurn(target);
	demoController.setDemoSteps(this->demoSteps, this->level.getRecordedDemoLength());
	if (target < overTurn) {
		overTurn = 0;
	}
}

void DemoRecorderLayer::showPausedDialog() {
	DialogLayer* layer = new GamePadStartDismissingDialog(this);
	layer->setTitle("Recording paused");

	layer->addDialogItem(new TextDialogItem(level.getInfo().title));
	layer->addDialogItem(new EmptyDialogItem(0.5f));
	if (level.isSuccessfullyOver()) {
		layer->addDialogItem(new CommandDialogItem("Save demo", [=] {
			promptSaveDemoAndExit(layer);
		}));
	}
	layer->addDialogItem(new CommandDialogItem("Continue", [=] {
		layer->dismiss();
	}));
	layer->addDialogItem(new CommandDialogItem("Restart recording", [=] {
		level = levelPrototype;
		turnPercent = 0.0f;
		setRandomSeed((unsigned int) core::MonotonicTime::getCurrent());
		demoController.setDemoSteps(this->demoSteps, this->level.getRecordedDemoLength());
		drawer.levelReloaded();
		layer->dismiss();
	}));
	layer->addDialogItem(new CommandDialogItem(this->getNextReturnText(), [=] {
		layer->dismiss();
		this->dismiss();
	}));
	layer->addDialogItem(new EmptyDialogItem(0.5f));
	layer->addDialogItem(new CommandDialogItem("Settings", [=] {
		auto* settingslayer = new SettingsLayer(layer, static_cast<SapphireScene*>(getScene())->getSettings());
		settingslayer->show(getScene(), true);
	}));
	if (this->getDrawer().isSplittable()) {
		auto* splitcmd = new CommandDialogItem(this->getDrawer().splitTwoPlayers ? "Join screen" : "Split screen");
		splitcmd->setHandler([=] {
			this->getDrawer().splitTwoPlayers = !this->getDrawer().splitTwoPlayers;
			splitcmd ->setText(this->getDrawer().splitTwoPlayers ? "Join screen" : "Split screen");
			layer->invalidate();
		});
		layer->addDialogItem(splitcmd);
	}
	layer->show(getScene(), true);
}

void DemoRecorderLayer::onLevelEnded() {
	if (endResumed) {
		return;
	}
	DialogLayer* layer = new DialogLayer(this);
	layer->setTitle("Demo complete");
	if (level.isSuccessfullyOver()) {
		layer->addDialogItem(new CommandDialogItem("Save demo", [=] {
			promptSaveDemoAndExit(layer);
		}));
	} else {
		//TODO failed to complete
	}
	layer->addDialogItem(new CommandDialogItem("Continue", [=] {
		endResumed = true;
		layer->dismiss();
	}));
	layer->addDialogItem(new CommandDialogItem(this->getNextReturnText(), [=] {
		layer->dismiss();
		this->dismiss();
	}));
	layer->show(getScene(), true);

}

void DemoRecorderLayer::drawImpl(float displaypercent) {
	setPaddings(levelController.getPaddings());
	DemoLayer::drawImpl(displaypercent);
}

void DemoRecorderLayer::sizeChanged(const core::WindowSize& size) {
	DemoLayer::sizeChanged(size);
	levelController.onSizeChanged(size);

	spaceRect = Rectangle { 0, 0, (float) size.pixelSize.width(), (float) size.pixelSize.height() }.inset(size.pixelSize / 8.0f);
}

void DemoRecorderLayer::onGamePadAttached(GamePad* gamepad) {
}
void DemoRecorderLayer::onGamePadDetached(GamePad* gamepad) {
	if (hasInputFocus()) {
		showPausedDialog();
	}
}

void DemoRecorderLayer::onTimeChanged(const core::time_millis& time, const core::time_millis& previous) {
	if (demoController.isControlsVisible()) {
		levelController.clearInput();
	}
	DemoLayer::onTimeChanged(time, previous);
}

void DemoRecorderLayer::drawHud(float displaypercent) {
	hudDrawer.draw(level, displaypercent, &levelController);
}

void DemoRecorderLayer::setScene(Scene* scene) {
	DemoLayer::setScene(scene);
	auto* ss = static_cast<SapphireScene*>(scene);
	levelController.setScene(ss);

	if (gamepadContext != nullptr) {
		gamepadContext->addGamePadStateListener(this);
	}
}
void DemoRecorderLayer::applyDemoSteps(const char* steps, unsigned int stepslength) {
	DemoPlayer player;
	player.play(steps, stepslength);
	while (!player.isOver()) {
		for (unsigned int i = 0; i < level.getPlayerCount(); ++i) {
			player.applyTurnForPlayer(level, i);
		}
		goToNextTurn();
	}
}
void DemoRecorderLayer::setRandomSeed(unsigned int seed) {
	level.setRandomSeed(seed);
	levelPrototype.setRandomSeed(seed);
}

} // namespace userapp
