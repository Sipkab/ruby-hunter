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
 * LeaderboardsDialog.h
 *
 *  Created on: 2017. aug. 6.
 *      Author: sipka
 */

#ifndef JNI_TEST_SAPPHIRE_DIALOGS_LEADERBOARDSDIALOG_H_
#define JNI_TEST_SAPPHIRE_DIALOGS_LEADERBOARDSDIALOG_H_

#include <framework/utils/ArrayList.h>
#include <gen/fwd/types.h>
#include <gen/types.h>
#include <sapphire/community/CommunityConnection.h>
#include <sapphire/dialogs/DialogLayer.h>
#include <sapphire/dialogs/Refreshable.h>

namespace userapp {
using namespace rhfw;

class SelectorEnumDialogItem;
class SapphireLevelDescriptor;

class LeaderboardsDialog: public DialogLayer, private CommunityConnection::StateListener, public Refreshable {
public:
	static const unsigned int LEADERBOARD_COUNT = 3;
private:
	static const int STATE_UNQUERIED = 0;
	static const int STATE_DOWNLOADING = 1;
	static const int STATE_DOWNLOAD_FAILED = 2;
	static const int STATE_SUCCESS = 3;
	static const int STATE_DOWNLOAD_REQUESTED = 4;

	class LeaderboardInfo {
	public:
		ArrayList<CommunityConnection::LeaderboardEntry> entries;
		int32 userIndex;
		uint32 userScore;
		int32 userPosition;
		PlayerDemoId userDemoId;
		uint32 totalCount;
	};
	const SapphireLevelDescriptor* descriptor;
	const char* labels[LEADERBOARD_COUNT];
	unsigned int supportedLeaderboardCount = 0;
	SapphireLeaderboards supportedTypes[LEADERBOARD_COUNT];

	LeaderboardInfo* infos[LEADERBOARD_COUNT] { nullptr };

	int states[LEADERBOARD_COUNT] { };

	SelectorEnumDialogItem* selectorItem = nullptr;

	CommunityConnection::LeaderboardDownloadedListener::Listener leaderboardListener;

	virtual void onLoggedIn(CommunityConnection* connection) override;
	virtual void onDisconnected(CommunityConnection* connection) override;
	virtual void onConnectionFailed(CommunityConnection* connection) override;

	ValueCommandDialogItem* createLeaderboardEntryDialogItem(LeaderboardInfo* info, unsigned int position, unsigned int score, const FixedString& name, PlayerDemoId demoid);
	void putDialogItems(SapphireScene* scene, bool autodownloadenabled = true);

	void tryDownloadingStatisticsForSelected();

protected:
	virtual void onSceneSizeInitialized() override;
public:

	LeaderboardsDialog(SapphireUILayer* parent, const SapphireLevelDescriptor* descriptor, SapphireLeaderboards leaderboardtypes);
	~LeaderboardsDialog();

	virtual void setScene(Scene* scene) override;

	virtual void refresh() override;

	virtual void sizeChanged(const rhfw::core::WindowSize& size) override;
};

} // namespace userapp

#endif /* JNI_TEST_SAPPHIRE_DIALOGS_LEADERBOARDSDIALOG_H_ */
