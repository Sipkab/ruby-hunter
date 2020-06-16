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
 * LevelSuccesLayer.cpp
 *
 *  Created on: 2016. apr. 28.
 *      Author: sipka
 */

#include <framework/layer/Layer.h>
#include <framework/utils/FixedString.h>
#include <framework/utils/utility.h>
#include <sapphire/dialogs/LevelFailedLayer.h>
#include <sapphire/DemoLayer.h>
#include <sapphire/level/Demo.h>
#include <sapphire/level/DemoInfo.h>
#include <sapphire/level/Level.h>
#include <sapphire/PlayerLayer.h>
#include <sapphire/SapphireUILayer.h>
#include <sapphire/DemoReplayerLayer.h>

namespace userapp {
using namespace rhfw;

LevelFailedLayer::LevelFailedLayer(PlayerLayer* parent)
		: DialogLayer(parent) {
	setTitle("Level failed");

	if (parent->isSuspendedLevel()) {
		addDialogItem(new CommandDialogItem("Restart suspended level", [this] {
			static_cast<PlayerLayer*>(getParent())->restartSuspendedLevel();
			/*do not dismiss parent dialog*/
			DialogLayer::dismiss();
		}));
	}

	addDialogItem(new CommandDialogItem("Restart level", [this] {
		static_cast<PlayerLayer*>(getParent())->restartLevel();
		/*do not dismiss parent dialog*/
		DialogLayer::dismiss();
	}));

	if (parent->getDescriptor() != nullptr && !parent->isSuspendedLevel() && parent->getDescriptor()->hasSuspendedGame) {
		addDialogItem(new CommandDialogItem("Restart suspended level", [this] {
			static_cast<PlayerLayer*>(getParent())->restartSuspendedLevel();
			/*do not dismiss parent dialog*/
			DialogLayer::dismiss();
		}));
	}

	addDialogItem(new CommandDialogItem(parent->getNextReturnText(), [this] {
		this->dismiss();
	}));

	auto& level = parent->getLevel();
	if (level.getDemoCount() > 0) {
		addDialogItem(new EmptyDialogItem(0.5f));

		for (unsigned int i = 0; i < level.getDemoCount(); ++i) {
			auto* demo = level.getDemo(i);

			addDialogItem(new CommandDialogItem(FixedString { "View: " } + demo->info.title, [=] {
				auto* desc = static_cast<PlayerLayer*>(getParent())->getDescriptor();
				Level level;
				level.loadLevel(desc->getFileDescriptor());
				static_cast<SapphireScene*>(getScene())->loadCustomDemos(level, desc);
				auto* layer = new DemoReplayerLayer(getParent()->getParent(), desc, util::move(level), i);
				layer->setSelectorLayer(parent->getSelectorLayer());

				this->dismiss();
				/*parent automatically dismissed*/
				layer->show(getScene());
			}));
		}
	}
}

void LevelFailedLayer::dismiss() {
	DialogLayer::dismiss();
	//carefully edit this function, its not called when "Restart level" is selected
	getParent()->dismiss();
}
}  // namespace userapp

