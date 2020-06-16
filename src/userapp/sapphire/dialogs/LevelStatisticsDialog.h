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
 * LevelStatisticsDialog.h
 *
 *  Created on: 2017. aug. 2.
 *      Author: sipka
 */

#ifndef JNI_TEST_SAPPHIRE_DIALOGS_LEVELSTATISTICSDIALOG_H_
#define JNI_TEST_SAPPHIRE_DIALOGS_LEVELSTATISTICSDIALOG_H_

#include <framework/utils/FixedString.h>
#include <sapphire/level/Level.h>
#include <sapphire/dialogs/DialogLayer.h>
#include <sapphire/SapphireScene.h>
#include <sapphire/dialogs/Refreshable.h>

namespace userapp {
using namespace rhfw;

class SapphireLevelDescriptor;
class LevelStatistics;
class SelectorEnumDialogItem;
class SapphireScene;

class LevelStatisticsDialog: public DialogLayer, private CommunityConnection::StateListener, public Refreshable {
public:
	static void putStatItems(DialogLayer* dialog, const LevelStatistics& stats);
private:
	static const int GLOBALSTAT_UNQUERIED = -1;
	static const int GLOBALSTAT_DOWNLOADING = -2;
	static const int GLOBALSTAT_FAILED = -3;

	const SapphireLevelDescriptor* descriptor;
	unsigned int currentType;
	LevelStatistics recentStats;
	LevelStatistics globalStats;
	int globalStatPlayCount = GLOBALSTAT_UNQUERIED;
	FixedString recentSteps;
	uint32 recentRandomSeed;

	SelectorEnumDialogItem* typeItem = nullptr;

	CommunityConnection::StatisticsDownloadListener::Listener statisticsListener;
	SapphireScene::LevelProgressListener::Listener levelProgressListener;

	void putDialogItems(SapphireScene* scene);
	void putStatItems(const LevelStatistics& stats);
	void putAverageStatItems(const LevelStatistics& stats, unsigned int playcount);

	void tryDownloadingStatistics();

	void putNoGlobalStatsItems();
	void putGlobalStatusItems();

	int putNonZeroStatItem(const char* name, unsigned int value);
	int putNonZeroFloatStatItem(const char* name, unsigned int value, unsigned int playcount);

	void addPlayLevelItem();

	virtual void onLoggedIn(CommunityConnection* connection) override;
	virtual void onDisconnected(CommunityConnection* connection) override;
	virtual void onConnectionFailed(CommunityConnection* connection) override;
protected:
	virtual void onSceneSizeInitialized() override;
public:
	LevelStatisticsDialog(SapphireUILayer* parent, const SapphireLevelDescriptor* descriptor);
	LevelStatisticsDialog(SapphireUILayer* parent, const SapphireLevelDescriptor* descriptor, const LevelStatistics& recentstats,
			FixedString recentsteps, uint32 recentrandomseed);
	~LevelStatisticsDialog();

	virtual void refresh() override;

	virtual void setScene(Scene* scene) override;
};

} // namespace userapp

#endif /* JNI_TEST_SAPPHIRE_DIALOGS_LEVELSTATISTICSDIALOG_H_ */
