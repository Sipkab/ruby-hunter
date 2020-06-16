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

#include <sapphire/dialogs/LevelSuccessLayer.h>
#include <sapphire/SapphireUILayer.h>
#include <sapphire/PlayerLayer.h>
#include <sapphire/dialogs/items/EditTextDialogItem.h>
#include <sapphire/SapphireScene.h>
#include <sapphire/dialogs/items/LevelRatingDialogItem.h>
#include <sapphire/dialogs/ShowDemoNameDialog.h>
#include <sapphire/dialogs/LeaderboardsDialog.h>

namespace userapp {
using namespace rhfw;

LevelSuccessLayer::LevelSuccessLayer(PlayerLayer* parent, const SapphireLevelDescriptor* next)
		: DialogLayer(parent), next(next) {
	setTitle("Level complete");
	auto* ss = static_cast<SapphireScene*>(parent->getScene());

	if (next != nullptr) {
		addDialogItem(new CommandDialogItem(FixedString { "Play next: " } + next->title, [this, parent, next] {
			/*do not dismiss parent dialog*/
			DialogLayer::dismiss();
			parent->setLevel(next);
		}));
	}
	auto* descriptor = parent->getDescriptor();
	ASSERT(descriptor != nullptr);

	addDialogItem(new CommandDialogItem(parent->getNextReturnText(), [this] {
		this->dismiss();
	}));
	auto* savedemoitem = new CommandDialogItem("Save as demo");
	savedemoitem->setHandler([=] {
		ShowDemoNameDialog(this, [=](const FixedString& name) {
					ss->saveDemo(
							descriptor,
							name,
							parent->getLevel().getOriginalRandomSeed(),
							parent->getLevel().getRecordedDemo(),
							parent->getLevel().getRecordedDemoLength());
					this->removeDialogItem(savedemoitem);
					delete savedemoitem;
				});
	});
	addDialogItem(savedemoitem);
	addDialogItem(new CommandDialogItem("Level statistics", [=] {
		ss->showStatisticsDialogForLevel(this, parent->getDescriptor(), parent->getLevelStatistics(),
				parent->getLevel().getRecordedDemoString(), parent->getLevel().getOriginalRandomSeed());
	}));
	if (descriptor->hasLeaderboards()) {
		addDialogItem(new CommandDialogItem("Leaderboards", [=] {
			ss->showLeaderboardsDialogForLevel(this, descriptor);
		}));
	}

	if (descriptor->isRateable()) {
		addDialogItem(new LevelRatingDialogItem(descriptor));
	}
	if (parent->isSuspendedLevel()) {
		auto* emptysuspended = new EmptyDialogItem(0.25f);
		auto* delsuspended = new CommandDialogItem("Delete suspended game");
		delsuspended->setHandler([=] {
			ss->deleteSuspendedLevel(descriptor);
			this->queueRemoveDialogItem(emptysuspended);
			this->queueRemoveDialogItem(delsuspended);
			this->finishRemoveDialogItem();

			delete emptysuspended;
			delete delsuspended;
		});

		addDialogItem(emptysuspended);
		addDialogItem(delsuspended);
	}
}

void LevelSuccessLayer::dismiss() {
	DialogLayer::dismiss();
	this->parent->dismiss();
}

void LevelSuccessLayer::setScene(Scene* scene) {
	DialogLayer::setScene(scene);
//	auto* ss = static_cast<SapphireScene*>(scene);
//	auto* parent = static_cast<PlayerLayer*>(getParent());
}

}  // namespace userapp

