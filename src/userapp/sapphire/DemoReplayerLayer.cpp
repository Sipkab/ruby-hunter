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
 * DemoReplayerLayer.cpp
 *
 *  Created on: 2016. dec. 22.
 *      Author: sipka
 */

#include <sapphire/DemoReplayerLayer.h>
#include <sapphire/dialogs/ParentDismisserDialogLayer.h>
#include <sapphire/dialogs/SettingsLayer.h>
#include <sapphire/SapphireScene.h>
#include <sapphire/PlayerLayer.h>
#include <sapphire/LevelEditorLayer.h>
#include <sapphire/sapphireconstants.h>
#include <sapphire/dialogs/items/EditTextDialogItem.h>
#include <sapphire/DemoRecorderLayer.h>
#include <framework/utils/utility.h>

#include <stdio.h>

#include <gen/configuration.h>

namespace userapp {
using namespace rhfw;

template<typename T>
static T stringToNumber(const char* str, unsigned int len, T defaultvalue) {
	if (len == 0) {
		return util::move(defaultvalue);
	}
	T result = str[0] - '0';
	for (unsigned int i = 1; i < len; ++i) {
		result = result * 10 + (str[i] - '0');
	}
	return result;
}
template<typename T>
static T stringToNumber(const FixedString& fs, T defaultvalue) {
	return stringToNumber((const char*) fs, fs.length(), util::move(defaultvalue));
}

DemoReplayerLayer::DemoReplayerLayer(SapphireUILayer* parent, const SapphireLevelDescriptor* descriptor, Level level,
		unsigned int demoindex)
		: DemoLayer(parent, descriptor, util::move(level)), demoIndex(demoindex) {

	const Demo* demo = this->levelPrototype.getDemo(demoIndex);
	this->level.setRandomSeed(demo->randomseed);
	this->levelPrototype.setRandomSeed(demo->randomseed);

	this->demoController.setDemoSteps(demo->moves, demo->moves.length());

	this->returnText = "Return to demo replayer";
}
DemoReplayerLayer::DemoReplayerLayer(SapphireUILayer* parent, const SapphireLevelDescriptor* descriptor, Level level, FixedString steps,
		uint32 randomseed)
		: DemoLayer(parent, descriptor, util::move(level)) {
	this->level.setRandomSeed(randomseed);
	this->levelPrototype.setRandomSeed(randomseed);

	this->demoController.setDemoSteps(util::move(steps));

	this->returnText = "Return to demo replayer";
}
DemoReplayerLayer::~DemoReplayerLayer() {
}

void DemoReplayerLayer::showPausedDialog() {
	DialogLayer* layer = new DialogLayer(this);
	layer->setTitle("Demo paused");

	layer->addDialogItem(new TextDialogItem(level.getInfo().title));
	if (demoInfo != nullptr) {
		layer->addDialogItem(new TextDialogItem(demoInfo));
	}
	if (demoIndex >= 0) {
		layer->addDialogItem(new TextDialogItem(FixedString { "Demo: " } + getDemo()->info.title));
	}

	layer->addDialogItem(new EmptyDialogItem(0.5f));

	layer->addDialogItem(new CommandDialogItem("Continue", [=] {
		layer->dismiss();
	}));
	if (playLevelAllowd) {
		layer->addDialogItem(new CommandDialogItem("Play level", [=] {
			auto* parent = this->getParent();
			Level loaded = levelPrototype;
			loaded.setRandomSeed((unsigned int) core::MonotonicTime::getCurrent());
			PlayerLayer* plrlayer = new PlayerLayer(parent, descriptor, util::move(loaded));
			plrlayer->setSelectorLayer(this->getSelectorLayer());
			plrlayer->setEditorLayer(editorLayer);

			layer->dismiss();
			this->dismiss();
			plrlayer->show(getScene(), false);
		}));
	}

	layer->addDialogItem(new CommandDialogItem(this->getNextReturnText(), [=] {
		layer->dismiss();
		this->dismiss();
	}));
	layer->addDialogItem(new CommandDialogItem("Restart demo", [=] {
		restartDemo();
		layer->dismiss();
	}));

	layer->addDialogItem(new CommandDialogItem("Jump to time", [=] {
		DialogLayer* timelayer = new DialogLayer(layer);
		timelayer->setTitle("Jump to time");
		timelayer->addDialogItem(new TextDialogItem(FixedString {"Demo length: "}+
						numberToString(demoController.getDemoStepsLength() / level.getPlayerCount())));

		timelayer->addDialogItem(new EmptyDialogItem(0.5f));
		auto* timeitem = new EditTextDialogItem("Jump to:", "", 4.0f);
		timeitem->setNumericOnly(true);
		timeitem->setReturnHandler([=] {
					if(jumpTo(timelayer, timeitem)) {
						/*dismiss both and keep watching*/
						timelayer->dismiss();
						layer->dismiss();
					}
					return true;
				});
		timelayer->addDialogItem(timeitem);
		timelayer->addDialogItem(new EmptyDialogItem(0.5f));
		timelayer->addDialogItem(new CommandDialogItem( "Go", [=] {
							if(jumpTo(timelayer, timeitem)) {
								/*dismiss both and keep watching*/
								timelayer->dismiss();
								layer->dismiss();
							}
						}));
		timelayer->show(getScene(), true);
		timelayer->setHighlighted(timeitem);
	}));

	layer->addDialogItem(new EmptyDialogItem(0.5f));

	if (demoIndex >= 0 && (this->getDemo()->userDemo || editorLayer != nullptr)) {
		layer->addDialogItem(new CommandDialogItem("Delete demo", [=] {
			if(editorLayer != nullptr) {
				editorLayer->removeDemo(demoIndex);
			} else {
				static_cast<SapphireScene*>(getScene())->removeDemo(this->getLevel(), demoIndex);
			}
			layer->dismiss();
			this->dismiss();
		}));
	}

	layer->addDialogItem(new CommandDialogItem("Settings", [=] {
		auto* setlayer = new SettingsLayer(layer, static_cast<SapphireScene*>(getScene())->getSettings());
		setlayer->show(getScene(), true);
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

#if RHFW_DEBUG
	layer->addDialogItem(new CommandDialogItem("Record demo from this point", [=] {
		Level l;
		l.loadLevel(this->descriptor->getFileDescriptor());
		auto* recorder = new DemoRecorderLayer(this->getParent(), this->descriptor, util::move(l));
		recorder->setSelectorLayer(selectorLayer);
		recorder->setRandomSeed(this->level.getOriginalRandomSeed());
		recorder->applyDemoSteps(this->demoController.getDemoSteps(), this->level.getTurn() * this->descriptor->playerCount);
		layer->dismiss();
		this->dismiss();
		recorder->show(getScene());
	}));
#endif /* RHFW_DEBUG */
	layer->show(getScene(), true);
}

void DemoReplayerLayer::goToNextTurn() {
	if ((level.getTurn() + 1) * level.getPlayerCount() <= demoController.getDemoStepsLength()) {
		//if (!demo.isOver()) {
		DemoPlayer::playMoves(demoController.getDemoSteps() + level.getTurn() * level.getPlayerCount(), 1, level);
		//demo.next(level);
	} else {
		//if demo is over, we still play some turns
		level.applyTurn();
	}
	if (overTurn == 0 && isOver()) {
		this->overTurn = level.getTurn();
	}
}

void DemoReplayerLayer::goToPreviousTurn(unsigned int target) {
	DemoLayer::goToPreviousTurn(target);
	if (target < overTurn) {
		overTurn = 0;
	}
}

bool DemoReplayerLayer::isOver() {
	return level.isOver() || level.getTurn() * level.getPlayerCount() >= demoController.getDemoStepsLength();
}

int DemoReplayerLayer::getOverTurns() {
	if (isOver()) {
		return overTurn;
	}
	return demoController.getDemoStepsLength() / level.getPlayerCount();
}

void DemoReplayerLayer::restartDemo() {
	level = levelPrototype;
	turnPercent = 0.0f;
}

void DemoReplayerLayer::onLevelEnded() {
#if SAPPHIRE_SCREENSHOT_MODE
	return;
#endif /* SAPPHIRE_SCREENSHOT_MODE */

	ParentDismisserDialogLayer* layer = new ParentDismisserDialogLayer(this);
	layer->setTitle("Demo over");

	layer->addDialogItem(new TextDialogItem(level.getInfo().title));
	if (demoInfo != nullptr) {
		layer->addDialogItem(new TextDialogItem(demoInfo));
	}

	layer->addDialogItem(new EmptyDialogItem { 0.5f });

	if (playLevelAllowd) {
		layer->addDialogItem(new CommandDialogItem { "Play level", [=] {
			auto* parent = this->getParent();
			layer->dismiss();
			Level loaded = levelPrototype;
			loaded.setRandomSeed((unsigned int) core::MonotonicTime::getCurrent());
			auto* plrlayer = new PlayerLayer(parent, descriptor, util::move(loaded));
			plrlayer->setSelectorLayer(this->getSelectorLayer());
			plrlayer->setEditorLayer(editorLayer);
			plrlayer->show(getScene(), false);
		} });
	}

	layer->addDialogItem(new CommandDialogItem { "Replay demo", [=] {
		this->restartDemo();
		layer->dismissKeepParent();
	} });

	layer->addDialogItem(new CommandDialogItem { this->getNextReturnText(), [=] {
		layer->dismiss();
	} });
	layer->show(getScene(), true);
}

void DemoReplayerLayer::setScene(Scene* scene) {
	DemoLayer::setScene(scene);
}

bool DemoReplayerLayer::jumpTo(DialogLayer* layer, EditTextDialogItem* timeitem) {
	int time = stringToNumber(timeitem->getContent(), timeitem->getContentLength(), -1);
	unsigned int maxtime = demoController.getDemoStepsLength() / level.getPlayerCount();
	if (time < 0 || time > maxtime) {
		DialogLayer* info = new DialogLayer(layer);
		info->setTitle("Invalid turn");
		info->addDialogItem(new TextDialogItem(FixedString { "Enter a turn value between 0 - " } + numberToString(maxtime)));
		info->addDialogItem(new EmptyDialogItem(0.5f));
		info->addDialogItem(new CommandDialogItem("Ok", [=] {
			layer->setHighlighted(timeitem);
			info->dismiss();
		}));
		info->show(getScene(), true);
		return false;
	}
	goToTurn(time);
	return true;
}
} // namespace userapp

