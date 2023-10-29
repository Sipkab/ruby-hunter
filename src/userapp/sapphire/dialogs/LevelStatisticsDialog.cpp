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
 * LevelStatisticsDialog.cpp
 *
 *  Created on: 2017. aug. 2.
 *      Author: sipka
 */

#include <sapphire/dialogs/items/SelectorEnumDialogItem.h>
#include <sapphire/dialogs/items/ValueTextDialogItem.h>
#include <sapphire/dialogs/items/BusyIndicatorDialogItem.h>
#include <sapphire/dialogs/LevelStatisticsDialog.h>
#include <sapphire/level/Level.h>
#include <sapphire/SapphireScene.h>
#include <sapphire/sapphireconstants.h>
#include <sapphire/DemoReplayerLayer.h>
#include <sapphire/PlayerLayer.h>

#include <stdio.h>

#define STAT_TIME "Time spent"
#define STAT_STEPS_TAKEN "Steps taken"

#define STAT_EMERALD_COLLECTED "Emeralds collected"
#define STAT_SAPPHIRE_COLLECTED "Sapphires collected"
#define STAT_RUBY_COLLECTED "Rubys collected"
#define STAT_CITRINE_COLLECTED "Citrines collected"
#define STAT_KEYS_COLLECTED "Keys collected"
#define STAT_DIRT_MINED "Earth mined"

#define STAT_TIMEBOMB_COLLECTED "Timebombs collected"
#define STAT_TIMEBOMB_SET "Timebombs set"

#define STAT_SAPPHIRES_BROKEN "Sapphires broken"
#define STAT_CITRINES_BROKEN "Citrines broken"

#define STAT_ITEMS_CONVERTED "Items converted"
#define STAT_LASERS_FIRED "Lasers fired"
#define STAT_WHEELS_TURNED "Wheels turned"

#define STAT_SAFES_OPENED "Safes opened"
#define STAT_BAGS_OPENED "Bags opened"

#define STAT_GROUP_SPACE 0.25f

#define NO_INTERESTING_STATS "No interesting stats available."

#define CONVERT_TO_TIMES_STRING(value) (value == 1 ? "1 time" : numberToString(value) + " times")
#define TEXT_REFRESH_STATS "Refresh"

namespace userapp {

using namespace rhfw;
static const unsigned int TYPE_RECENT = 0;
static const unsigned int TYPE_GLOBAL = 1;
static const unsigned int TYPE_GLOBAL_AVERAGE = 2;
static const unsigned int STATISTICS_TYPE_COUNT = 3;
static const unsigned int STATISTICS_RECENT_ONLY_TYPE_COUNT = 1;
static const char* STATISTICS_TYPES[STATISTICS_TYPE_COUNT] = { "Recent", "Total", "Average" };

LevelStatisticsDialog::LevelStatisticsDialog(SapphireUILayer* parent, const SapphireLevelDescriptor* descriptor,
		const LevelStatistics& recentstats, FixedString recentsteps, uint32 recentrandomseed)
		: DialogLayer(parent), descriptor(descriptor), currentType(TYPE_RECENT), recentStats(recentstats), recentSteps(
				util::move(recentsteps)), recentRandomSeed(recentrandomseed) {
	setTitle("Level statistics");
}
LevelStatisticsDialog::LevelStatisticsDialog(SapphireUILayer* parent, const SapphireLevelDescriptor* descriptor)
		: DialogLayer(parent), descriptor(descriptor), currentType(TYPE_RECENT), recentRandomSeed(0) {
	setTitle("Level statistics");
}

LevelStatisticsDialog::~LevelStatisticsDialog() {
}

void LevelStatisticsDialog::putDialogItems(SapphireScene* scene) {
	addDialogItem(new EmptyDialogItem(STAT_GROUP_SPACE));
	switch (currentType) {
		case TYPE_RECENT: {
			if (recentSteps.length() == 0) {
				addDialogItem(new TextDialogItem("No recent statistics available. Complete the level to record one."));
				addDialogItem(new EmptyDialogItem(STAT_GROUP_SPACE));
				addPlayLevelItem();
			} else {
				putStatItems(recentStats);
				addDialogItem(new CommandDialogItem("View replay", [=] {
					Level level;
					level.loadLevel(descriptor->getFileDescriptor());
					auto* replayer = new DemoReplayerLayer(this, descriptor, util::move(level), recentSteps, recentRandomSeed);
					replayer->setPlayLevelAllowed(false);
					replayer->show(scene);
				}));
			}
			break;
		}
		case TYPE_GLOBAL: {
			if (globalStatPlayCount == 0) {
				putNoGlobalStatsItems();
			} else if (globalStatPlayCount < 0) {
				putGlobalStatusItems();
			} else {
				putStatItems(globalStats);
				addDialogItem(new ValueTextDialogItem("Level completed", CONVERT_TO_TIMES_STRING(globalStatPlayCount)));
				addDialogItem(new EmptyDialogItem(STAT_GROUP_SPACE));
				addDialogItem(new CommandDialogItem( TEXT_REFRESH_STATS, [=] {
					tryDownloadingStatistics();
				}));
			}
			break;
		}
		case TYPE_GLOBAL_AVERAGE: {
			if (globalStatPlayCount == 0) {
				putNoGlobalStatsItems();
			} else if (globalStatPlayCount < 0) {
				putGlobalStatusItems();
			} else {
				putAverageStatItems(globalStats, globalStatPlayCount);
				addDialogItem(new ValueTextDialogItem("Level completed", CONVERT_TO_TIMES_STRING(globalStatPlayCount)));
				addDialogItem(new EmptyDialogItem(STAT_GROUP_SPACE));
				addDialogItem(new CommandDialogItem( TEXT_REFRESH_STATS, [=] {
					tryDownloadingStatistics();
				}));
			}
			break;
		}
		default: {
			break;
		}
	}
}
static int PutNonZeroStatItem(DialogLayer* dialog, const char* name, unsigned int value) {
	if (value == 0) {
		return 0;
	}
	dialog->addDialogItem(new ValueTextDialogItem(name, numberToString(value)));
	return 1;
}
static int PutNonZeroFloatStatItem(DialogLayer* dialog, const char* name, unsigned int value, unsigned int playcount) {
	if (value == 0) {
		return 0;
	}
	char buffer[64];
	snprintf(buffer, sizeof(buffer), "%.1f", (float) value / playcount);
	dialog->addDialogItem(new ValueTextDialogItem(name, buffer));
	return 1;
}
int LevelStatisticsDialog::putNonZeroStatItem(const char* name, unsigned int value) {
	return PutNonZeroStatItem(this, name, value);
}
int LevelStatisticsDialog::putNonZeroFloatStatItem(const char* name, unsigned int value, unsigned int playcount) {
	return PutNonZeroFloatStatItem(this, name, value, playcount);
}

#define PUT_EMPTY_IF_NONZERO(value) if(value) addDialogItem(new EmptyDialogItem(STAT_GROUP_SPACE))
#define PUT_EMPTY_IF_NONZERO_DIALOG(dialog, value) if(value) (dialog)->addDialogItem(new EmptyDialogItem(STAT_GROUP_SPACE))
void LevelStatisticsDialog::putStatItems(DialogLayer* dialog, const LevelStatistics& stats) {
	int times = 0;
	int gems = 0;
	int timebombs = 0;
	int broken = 0;
	int misc = 0;
	int open = 0;

	times |= PutNonZeroStatItem(dialog, STAT_TIME, stats.turns);
	times |= PutNonZeroStatItem(dialog, STAT_STEPS_TAKEN, stats.moveCount);
	PUT_EMPTY_IF_NONZERO_DIALOG(dialog, times);

	gems |= PutNonZeroStatItem(dialog, STAT_EMERALD_COLLECTED, stats.emeraldCollected);
	gems |= PutNonZeroStatItem(dialog, STAT_SAPPHIRE_COLLECTED, stats.sapphireCollected);
	gems |= PutNonZeroStatItem(dialog, STAT_RUBY_COLLECTED, stats.rubyCollected);
	gems |= PutNonZeroStatItem(dialog, STAT_CITRINE_COLLECTED, stats.citrineCollected);
	gems |= PutNonZeroStatItem(dialog, STAT_KEYS_COLLECTED, stats.keysCollected);
	gems |= PutNonZeroStatItem(dialog, STAT_DIRT_MINED, stats.dirtMined);
	PUT_EMPTY_IF_NONZERO_DIALOG(dialog, gems);

	timebombs |= PutNonZeroStatItem(dialog, STAT_TIMEBOMB_COLLECTED, stats.timeBombsCollected);
	timebombs |= PutNonZeroStatItem(dialog, STAT_TIMEBOMB_SET, stats.timeBombsSet);
	PUT_EMPTY_IF_NONZERO_DIALOG(dialog, timebombs);

	broken |= PutNonZeroStatItem(dialog, STAT_SAPPHIRES_BROKEN, stats.sapphiresBroken);
	broken |= PutNonZeroStatItem(dialog, STAT_CITRINES_BROKEN, stats.citrinesBroken);
	PUT_EMPTY_IF_NONZERO_DIALOG(dialog, broken);

	misc |= PutNonZeroStatItem(dialog, STAT_ITEMS_CONVERTED, stats.itemsConverted);
	misc |= PutNonZeroStatItem(dialog, STAT_LASERS_FIRED, stats.lasersFired);
	misc |= PutNonZeroStatItem(dialog, STAT_WHEELS_TURNED, stats.wheelsTurned);
	PUT_EMPTY_IF_NONZERO_DIALOG(dialog, misc);

	open |= PutNonZeroStatItem(dialog, STAT_SAFES_OPENED, stats.safesOpened);
	open |= PutNonZeroStatItem(dialog, STAT_BAGS_OPENED, stats.bagsOpened);
	PUT_EMPTY_IF_NONZERO_DIALOG(dialog, open);

	if ((times | gems | timebombs | broken | misc | open) == 0) {
		dialog->addDialogItem(new TextDialogItem(NO_INTERESTING_STATS));
		PUT_EMPTY_IF_NONZERO_DIALOG(dialog, 1);
	}
}
void LevelStatisticsDialog::putStatItems(const LevelStatistics& stats) {
	putStatItems(this, stats);
}
void LevelStatisticsDialog::putAverageStatItems(const LevelStatistics& stats, unsigned int playcount) {
	int times = 0;
	int gems = 0;
	int timebombs = 0;
	int broken = 0;
	int misc = 0;
	int open = 0;

	times |= putNonZeroFloatStatItem(STAT_TIME, stats.turns, playcount);
	times |= putNonZeroFloatStatItem(STAT_STEPS_TAKEN, stats.moveCount, playcount);
	PUT_EMPTY_IF_NONZERO(times);

	gems |= putNonZeroFloatStatItem(STAT_EMERALD_COLLECTED, stats.emeraldCollected, playcount);
	gems |= putNonZeroFloatStatItem(STAT_SAPPHIRE_COLLECTED, stats.sapphireCollected, playcount);
	gems |= putNonZeroFloatStatItem(STAT_RUBY_COLLECTED, stats.rubyCollected, playcount);
	gems |= putNonZeroFloatStatItem(STAT_CITRINE_COLLECTED, stats.citrineCollected, playcount);
	gems |= putNonZeroFloatStatItem(STAT_KEYS_COLLECTED, stats.keysCollected, playcount);
	gems |= putNonZeroFloatStatItem(STAT_DIRT_MINED, stats.dirtMined, playcount);
	PUT_EMPTY_IF_NONZERO(gems);

	timebombs |= putNonZeroFloatStatItem(STAT_TIMEBOMB_COLLECTED, stats.timeBombsCollected, playcount);
	timebombs |= putNonZeroFloatStatItem(STAT_TIMEBOMB_SET, stats.timeBombsSet, playcount);
	PUT_EMPTY_IF_NONZERO(timebombs);

	broken |= putNonZeroFloatStatItem(STAT_SAPPHIRES_BROKEN, stats.sapphiresBroken, playcount);
	broken |= putNonZeroFloatStatItem(STAT_CITRINES_BROKEN, stats.citrinesBroken, playcount);
	PUT_EMPTY_IF_NONZERO(broken);

	misc |= putNonZeroFloatStatItem(STAT_ITEMS_CONVERTED, stats.itemsConverted, playcount);
	misc |= putNonZeroFloatStatItem(STAT_LASERS_FIRED, stats.lasersFired, playcount);
	misc |= putNonZeroFloatStatItem(STAT_WHEELS_TURNED, stats.wheelsTurned, playcount);
	PUT_EMPTY_IF_NONZERO(misc);

	open |= putNonZeroFloatStatItem(STAT_SAFES_OPENED, stats.safesOpened, playcount);
	open |= putNonZeroFloatStatItem(STAT_BAGS_OPENED, stats.bagsOpened, playcount);
	PUT_EMPTY_IF_NONZERO(open);

	if ((times | gems | timebombs | broken | misc | open) == 0) {
		addDialogItem(new TextDialogItem(NO_INTERESTING_STATS));
		PUT_EMPTY_IF_NONZERO(1);
	}
}

void LevelStatisticsDialog::refresh() {
	//remove everything but the type
	if (typeItem == nullptr) {
		return;
	}
	bool typehighlight = hasHighlighted();
	this->queueRemoveDialogItem(typeItem);
	this->clearDialogItems();

	addDialogItem(typeItem);

	putDialogItems(static_cast<SapphireScene*>(getScene()));
	relayout();

	if (typehighlight) {
		setHighlighted(typeItem);
	}
}

void LevelStatisticsDialog::addPlayLevelItem() {
	addDialogItem(new CommandDialogItem("Play level", [=] {
		Level level;
		level.loadLevel(descriptor->getFileDescriptor());
		auto* player = new PlayerLayer(this, descriptor, util::move(level));
		player->show(getScene());
	}));
}

void LevelStatisticsDialog::onLoggedIn(CommunityConnection* connection) {
	if (globalStatPlayCount < 0) {
		if (statisticsListener == nullptr) {
			statisticsListener = CommunityConnection::StatisticsDownloadListener::make_listener(
					[=](SapphireCommError error, const SapphireUUID& leveluuid, const LevelStatistics* stats, unsigned int playcount) {
						if(leveluuid != descriptor->uuid) {
							return;
						}
						switch (error) {
							case SapphireCommError::NoError: {
								globalStatPlayCount = playcount;
								globalStats = *stats;
								break;
							}
							case SapphireCommError::NoStatsYet: {
								globalStatPlayCount = 0;
								break;
							}
							default: {
								globalStatPlayCount = GLOBALSTAT_FAILED;
								break;
							}
						}
						refresh();
					});
			connection->statisticsDownloadEvents += statisticsListener;
		}
		connection->sendGetStatistics(descriptor->uuid);
		globalStatPlayCount = GLOBALSTAT_DOWNLOADING;
		refresh();
	}
}

void LevelStatisticsDialog::onDisconnected(CommunityConnection* connection) {
	if (globalStatPlayCount < 0) {
		globalStatPlayCount = GLOBALSTAT_FAILED;
		refresh();
	}
}

void LevelStatisticsDialog::onConnectionFailed(CommunityConnection* connection) {
	onDisconnected(connection);
}

void LevelStatisticsDialog::tryDownloadingStatistics() {
	auto* ss = static_cast<SapphireScene*>(getScene());
	globalStatPlayCount = GLOBALSTAT_DOWNLOADING;
	ss->getConnection().addStateListenerAndConnect(ss, *this);
	refresh();
}

void LevelStatisticsDialog::putNoGlobalStatsItems() {
	addDialogItem(new TextDialogItem("No recorded statistics available for this level yet."));
	addDialogItem(new EmptyDialogItem(STAT_GROUP_SPACE));
	addPlayLevelItem();
	addDialogItem(new CommandDialogItem( TEXT_REFRESH_STATS, [=] {
		tryDownloadingStatistics();
	}));
}

void LevelStatisticsDialog::putGlobalStatusItems() {
	switch (globalStatPlayCount) {
		case GLOBALSTAT_DOWNLOADING: {
			addDialogItem(new BusyIndicatorDialogItem("Downloading community statistics"));
			break;
		}
		case GLOBALSTAT_UNQUERIED: {
			tryDownloadingStatistics();
			break;
		}
		case GLOBALSTAT_FAILED: {
			addDialogItem(new TextDialogItem("Failed to download statistics."));
			addDialogItem(new EmptyDialogItem(STAT_GROUP_SPACE));
			addDialogItem(new CommandDialogItem("Retry", [=] {
				tryDownloadingStatistics();
			}));
			break;
		}
		default: {
			THROW();
			break;
		}
	}
}

void LevelStatisticsDialog::setScene(Scene* scene) {
	DialogLayer::setScene(scene);

}

void LevelStatisticsDialog::onSceneSizeInitialized() {
	if (typeItem == nullptr) {
		auto* ss = static_cast<SapphireScene*>(getScene());
		if (descriptor->isEditable()) {
			typeItem = new SelectorEnumDialogItem(STATISTICS_TYPES, currentType, STATISTICS_RECENT_ONLY_TYPE_COUNT);
		} else {
			typeItem = new SelectorEnumDialogItem(STATISTICS_TYPES, currentType, STATISTICS_TYPE_COUNT);
		}

		typeItem->setSelectionListener([=](unsigned int selected) {
			currentType = selected;
			refresh();
		});
		addDialogItem(typeItem);

		putDialogItems(ss);
		relayout();

		levelProgressListener =
				SapphireScene::LevelProgressListener::make_listener(
						[=](const SapphireLevelDescriptor* descriptor, LevelState state, const FixedString& steps, uint32 randomseed, const LevelStatistics* stats) {
							if(this->descriptor != descriptor || stats == nullptr || state != LevelState::COMPLETED) {
								return;
							}
							this->recentStats += *stats;
							this->recentSteps = steps;
							this->recentRandomSeed = randomseed;
							if(currentType == TYPE_RECENT) {
								refresh();
							}
						});
		ss->levelProgressEvents += levelProgressListener;
	}
}

} // namespace userapp

