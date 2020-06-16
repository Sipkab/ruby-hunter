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
 * PausedLayer.cpp
 *
 *  Created on: 2016. apr. 27.
 *      Author: sipka
 */

#include <framework/layer/Layer.h>
#include <sapphire/dialogs/PausedLayer.h>
#include <sapphire/dialogs/SettingsLayer.h>
#include <sapphire/SapphireUILayer.h>
#include <sapphire/PlayerLayer.h>

namespace userapp {

PausedLayer::PausedLayer(PlayerLayer* parent)
		: GamePadStartDismissingDialog(parent) {
	setTitle("Game paused");

	addDialogItem(new TextDialogItem(parent->getLevelTitle()));

	addDialogItem(new EmptyDialogItem(0.5f));

	addDialogItem(new CommandDialogItem("Continue", [=] {
		this->dismiss();
	}));

	if (parent->isSuspendedLevel()) {
		addDialogItem(new CommandDialogItem("Restart suspended level", [=] {
			parent->restartSuspendedLevel();
			this->dismiss();
		}));
	}

	addDialogItem(new CommandDialogItem("Restart level", [=] {
		parent->restartLevel();
		this->dismiss();
	}));

	if (parent->getDescriptor() != nullptr && !parent->isSuspendedLevel() && parent->getDescriptor()->hasSuspendedGame
			&& !parent->isTestingLevel()) {
		addDialogItem(new CommandDialogItem { "Restart suspended level", [this] {
			static_cast<PlayerLayer*>(this->parent)->restartSuspendedLevel();
			/*do not dismiss parent dialog*/
			DialogLayer::dismiss();
		} });
	}

	addDialogItem(new CommandDialogItem(parent->getNextReturnText(), [=] {
		this->dismiss();
		this->parent->dismiss();
	}));
	if (parent->getDescriptor() != nullptr && !parent->isTestingLevel()) {
		addDialogItem(new EmptyDialogItem(0.25f));
		addDialogItem(new CommandDialogItem("Suspend level", [=] {
			bool res = static_cast<SapphireScene*>(getScene())->suspendLevel(parent->getDescriptor(), parent->getLevel());
			/*TODO if not succeeded*/
			this->dismiss();
			parent->dismiss();
		}));
	}

	addDialogItem(new EmptyDialogItem(0.5f));

	addDialogItem(new CommandDialogItem("Settings", [=] {
		auto* layer = new SettingsLayer(this, static_cast<SapphireScene*>(getScene())->getSettings());
		layer->show(getScene(), true);
	}));
	if (parent->getDrawer().isSplittable()) {
		auto* splitcmd = new CommandDialogItem(parent->getDrawer().splitTwoPlayers ? "Join screen" : "Split screen");
		splitcmd->setHandler([=] {
			parent->getDrawer().splitTwoPlayers = !parent->getDrawer().splitTwoPlayers;
			splitcmd ->setText(parent->getDrawer().splitTwoPlayers ? "Join screen" : "Split screen");
			this->invalidate();
		});
		addDialogItem(splitcmd);
	}
}

}  // namespace userapp
