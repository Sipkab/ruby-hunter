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
 * OverallStatisticsDialog.cpp
 *
 *  Created on: 2017. szept. 13.
 *      Author: sipka
 */

#include <gen/types.h>
#include <appmain.h>
#include <sapphire/dialogs/items/EmptyDialogItem.h>
#include <sapphire/dialogs/items/ValueTextDialogItem.h>
#include <sapphire/dialogs/LevelStatisticsDialog.h>
#include <sapphire/dialogs/OverallStatisticsDialog.h>
#include <sapphire/level/LevelStatistics.h>
#include <sapphire/SapphireScene.h>

namespace userapp {
using namespace rhfw;

OverallStatisticsDialog::OverallStatisticsDialog(SapphireUILayer* parent, const LevelStatistics& stats)
		: DialogLayer(parent) {
	setTitle("Lifetime statistics");

	LevelStatistics s = stats;
	s.turns = 0;
	s.moveCount = 0;

	auto* ss = static_cast<SapphireScene*>(parent->getScene());
	auto usercolor = ss->getUserDifficultyColor();
	addDialogItem(new ValueTextDialogItem("Skill level", difficultyColorToSkillLevelName(usercolor)));
	addDialogItem(new EmptyDialogItem(0.5f));

	setColors(usercolor);

	LevelStatisticsDialog::putStatItems(this, s);

}
OverallStatisticsDialog::~OverallStatisticsDialog() {
}

} // namespace userapp
