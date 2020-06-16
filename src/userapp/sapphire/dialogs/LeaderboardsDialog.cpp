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
 * LeaderboardsDialog.cpp
 *
 *  Created on: 2017. aug. 6.
 *      Author: sipka
 */

#include <gen/types.h>
#include <sapphire/dialogs/LeaderboardsDialog.h>
#include <sapphire/level/SapphireLevelDescriptor.h>
#include <sapphire/SapphireScene.h>
#include <sapphire/dialogs/items/SelectorEnumDialogItem.h>
#include <sapphire/dialogs/items/ValueTextDialogItem.h>
#include <sapphire/dialogs/items/BusyIndicatorDialogItem.h>
#include <sapphire/DemoReplayerLayer.h>
#include <sapphire/PlayerLayer.h>

namespace userapp {
using namespace rhfw;

static const SapphireLeaderboards LEADERBOARD_TYPES[LeaderboardsDialog::LEADERBOARD_COUNT] { SapphireLeaderboards::MostGems,
		SapphireLeaderboards::LeastTime, SapphireLeaderboards::LeastSteps, };
static const char* LEADERBOARD_TITLES[LeaderboardsDialog::LEADERBOARD_COUNT] { "Most gems", "Least time", "Least steps" };

class PlayerDemoDownloaderDialog: public DialogLayer, private CommunityConnection::StateListener {
	CommunityConnection::PlayerDemoDownloadedListener::Listener demoDownloadedListener;
	const SapphireLevelDescriptor* descriptor;
	PlayerDemoId demoId;
	FixedString playerName;

	virtual void onLoggedIn(CommunityConnection* connection) override {
		if (demoDownloadedListener == nullptr) {
			demoDownloadedListener =
					CommunityConnection::PlayerDemoDownloadedListener::make_listener(
							[=](const SapphireUUID& leveluuid, PlayerDemoId demoid, SapphireCommError error, const FixedString& steps, uint32 randomseed) {
								if (leveluuid != descriptor->uuid || demoid != this->demoId) {
									return;
								}
								switch (error) {
									case SapphireCommError::NoError: {
										if(this->showing) {
											auto* parent = getParent();
											this->dismiss();
											Level level;
											level.loadLevel(descriptor->getFileDescriptor());
											auto* replayer = new DemoReplayerLayer(parent, descriptor, util::move(level), steps, randomseed);
											if (playerName != nullptr) {
												replayer->setDemoInfo(FixedString {"Leaderboard replay for: "}+ playerName);
											} else {
												replayer->setDemoInfo("Leaderboard entry replay");
											}
											replayer->show(getScene());
										}
										break;
									}
									default: {
										auto* parent = getParent();
										this->dismiss();

										DialogLayer* info = new DialogLayer(parent);
										info->setTitle("Failure");
										info->addDialogItem(new TextDialogItem("Failed to download replay.\n(Server error)"));
										info->addDialogItem(new EmptyDialogItem(0.5f));
										info->addDialogItem(new CommandDialogItem("Back", [=] {
															info->dismiss();
														}));
										info->showDialog(getScene());
										break;
									}
								}
							});
			connection->playerDemoDownloadedEvents += demoDownloadedListener;
		}
		connection->sendGetDemo(descriptor->uuid, demoId);
	}
	virtual void onDisconnected(CommunityConnection* connection) override {
		auto* parent = getParent();
		this->dismiss();

		DialogLayer* info = new DialogLayer(parent);
		info->setTitle("Connection failure");
		info->addDialogItem(new TextDialogItem("Failed to download leaderboard replay.\n(Connection unsuccessful)"));
		info->addDialogItem(new EmptyDialogItem(0.5f));
		info->addDialogItem(new CommandDialogItem("Back", [=] {
			info->dismiss();
		}));
		info->showDialog(getScene());
	}
	virtual void onConnectionFailed(CommunityConnection* connection) override {
		onDisconnected(connection);
	}
public:
	PlayerDemoDownloaderDialog(SapphireUILayer* parent, const SapphireLevelDescriptor* descriptor, PlayerDemoId demoid)
			: DialogLayer(parent), descriptor(descriptor), demoId(demoid) {
		setTitle("Downloading replay");

		addDialogItem(new BusyIndicatorDialogItem("Downloading leaderboard entry replay"));
		addDialogItem(new EmptyDialogItem(0.5f));
		addDialogItem(new CommandDialogItem("Cancel", [=] {
			dismiss();
		}));
	}
	virtual void setScene(Scene* scene) override {
		DialogLayer::setScene(scene);

		auto* ss = static_cast<SapphireScene*>(scene);
		ss->getConnection().addStateListenerAndConnect(ss, *this);
	}
	virtual void sizeChanged(const rhfw::core::WindowSize& size) override {
		DialogLayer::sizeChanged(size);
	}

	void setPlayerName(FixedString name) {
		this->playerName = util::move(name);
	}
};
LeaderboardsDialog::LeaderboardsDialog(SapphireUILayer* parent, const SapphireLevelDescriptor* descriptor,
		SapphireLeaderboards leaderboardtypes)
		: DialogLayer(parent), descriptor(descriptor) {
	setTitle("Leaderboards");

	addDialogItem(new TextDialogItem(descriptor->title));
	for (unsigned int i = 0; i < LEADERBOARD_COUNT; ++i) {
		if (HAS_FLAG(leaderboardtypes, LEADERBOARD_TYPES[i])) {
			labels[supportedLeaderboardCount] = LEADERBOARD_TITLES[i];
			supportedTypes[supportedLeaderboardCount] = LEADERBOARD_TYPES[i];
			++supportedLeaderboardCount;
		}
	}

	ASSERT(supportedLeaderboardCount != 0) << leaderboardtypes;

}
LeaderboardsDialog::~LeaderboardsDialog() {
	for (auto&& l : infos) {
		delete l;
	}
}

void LeaderboardsDialog::onLoggedIn(CommunityConnection* connection) {
	if (leaderboardListener == nullptr) {
		leaderboardListener = CommunityConnection::LeaderboardDownloadedListener::make_listener(
				[=](const SapphireUUID& leveluuid, SapphireLeaderboards type, SapphireCommError error,
						const ArrayList<CommunityConnection::LeaderboardEntry>* entries, int32 userindex,
						uint32 userscore, int32 userposition, PlayerDemoId userdemoid, uint32 totalcount) {
					if(leveluuid != descriptor->uuid) {
						return;
					}
					for (unsigned int i = 0; i < supportedLeaderboardCount; ++i) {
						if(supportedTypes[i] == type) {
							states[i] = STATE_SUCCESS;
							switch (error) {
								case SapphireCommError::NoError: {
									LeaderboardInfo* info = infos[i];
									if(info == nullptr) {
										infos[i] = info = new LeaderboardInfo();
									} else {
										infos[i]->entries.clear();
									}
									info->userIndex = userindex;
									info->userScore = userscore;
									info->userPosition = userposition;
									info->userDemoId = userdemoid;
									info->totalCount = totalcount;
									for (unsigned int i = 0; i < entries->size(); ++i) {
										info->entries.add(new CommunityConnection::LeaderboardEntry((*entries)[i]));
									}
									break;
								}
								default: {
									if(infos[i] != nullptr) {
										delete infos[i];
										infos[i] = nullptr;
									}
									break;
								}
							}
							if(selectorItem->getSelected() == i) {
								refresh();
							}
							break;
						}
					}
				});
		connection->leaderboardDownloadedEvents += leaderboardListener;
	}

	if (states[selectorItem->getSelected()] == STATE_DOWNLOAD_REQUESTED || states[selectorItem->getSelected()] == STATE_DOWNLOAD_FAILED) {
		connection->sendGetLeaderboards(descriptor->uuid, supportedTypes[selectorItem->getSelected()]);
		states[selectorItem->getSelected()] = STATE_DOWNLOADING;
		refresh();
	}
	for (unsigned int i = 0; i < supportedLeaderboardCount; ++i) {
		if (states[i] == STATE_DOWNLOAD_FAILED) {
			connection->sendGetLeaderboards(descriptor->uuid, supportedTypes[i]);
			states[i] = STATE_DOWNLOADING;
		}
	}
}

void LeaderboardsDialog::onDisconnected(CommunityConnection* connection) {
	bool changed = false;
	for (unsigned int i = 0; i < supportedLeaderboardCount; ++i) {
		if (states[i] == STATE_DOWNLOADING || states[i] == STATE_DOWNLOAD_REQUESTED) {
			states[i] = STATE_DOWNLOAD_FAILED;
			changed = i == selectorItem->getSelected();
		}
	}
	if (changed) {
		refresh();
	}
}

void LeaderboardsDialog::onConnectionFailed(CommunityConnection* connection) {
	onDisconnected(connection);
}

void LeaderboardsDialog::setScene(Scene* scene) {
	DialogLayer::setScene(scene);
}

void LeaderboardsDialog::onSceneSizeInitialized() {
	if (selectorItem == nullptr) {
		auto* ss = static_cast<SapphireScene*>(getScene());
		selectorItem = new SelectorEnumDialogItem(labels, 0, supportedLeaderboardCount);
		selectorItem->setSelectionListener([=](unsigned int selected) {
			refresh();
		});
		addDialogItem(selectorItem);

		putDialogItems(ss);
		relayout();
	}
}

void LeaderboardsDialog::sizeChanged(const rhfw::core::WindowSize& size) {
	DialogLayer::sizeChanged(size);
}

ValueCommandDialogItem* LeaderboardsDialog::createLeaderboardEntryDialogItem(LeaderboardInfo* info, unsigned int position,
		unsigned int score, const FixedString& name, PlayerDemoId demoid) {
	auto* item =
			new ValueCommandDialogItem(numberToString(position) + ". " + name,
					[=] {
						PlayerDemoDownloaderDialog* demodialog = new PlayerDemoDownloaderDialog(this, descriptor, demoid);
						if (name.length() > 0) {
							demodialog->setPlayerName(name);
						}
						demodialog->showDialog(getScene());
					});
	item->setValue(numberToString(score));
	return item;
}

void LeaderboardsDialog::putDialogItems(SapphireScene* scene, bool autodownloadenabled) {
	addDialogItem(new EmptyDialogItem(0.25f));
	auto* info = infos[selectorItem->getSelected()];
	if (info != nullptr && !info->entries.isEmpty()) {
		unsigned int position = 0;
		rhfw::uint64 prevscore = 0xFFFFFFFFFFFFFFFF;
		unsigned int addedcount = info->entries.size();
		for (unsigned int i = 0; i < info->entries.size(); ++i) {
			auto&& e = info->entries[i];
			if (e.score != prevscore) {
				++position;
				prevscore = e.score;
			}
			auto&& fsnametouse = (i == info->userIndex ? scene->getCurrentUserName() : e.userName);
			const char* name = fsnametouse.length() == 0 ? SAPPHIRE_PLAYER_PLACEHOLDER_NAME : (const char*) fsnametouse;
			auto* item = createLeaderboardEntryDialogItem(info, position, e.score, name, e.demoId);
			if (i == info->userIndex) {
				ASSERT(position == info->userPosition) << "Position mismatch: " << position << " - " << info->userPosition;
				item->setTextColor(Color { 1, 1, 1, 1 });
			}
			addDialogItem(item);
		}
		if (info->userIndex >= info->entries.size()) {
			if (info->userPosition > position) {
				addDialogItem(new EmptyDialogItem(0.4f));
			}
			auto* item = createLeaderboardEntryDialogItem(info, info->userPosition, info->userScore, scene->getCurrentUserName(),
					info->userDemoId);
			item->setTextColor(Color { 1, 1, 1, 1 });
			addDialogItem(item);
			++addedcount;
		}
		if (addedcount < info->totalCount) {
			addDialogItem(new EmptyDialogItem(0.25f));
			char buffer[256];
			sprintf(buffer, "And %u more...", info->totalCount - addedcount);
			addDialogItem(new TextDialogItem(buffer));
		}
		addDialogItem(new EmptyDialogItem(0.5f));
		addDialogItem(new CommandDialogItem("Refresh", [=] {
			tryDownloadingStatisticsForSelected();
		}));
	} else {
		switch (states[selectorItem->getSelected()]) {
			case STATE_UNQUERIED: {
				if (autodownloadenabled) {
					tryDownloadingStatisticsForSelected();
				}
				break;
			}
			case STATE_DOWNLOAD_REQUESTED:
			case STATE_DOWNLOADING: {
				addDialogItem(new BusyIndicatorDialogItem("Downloading leaderboard data"));
				break;
			}
			case STATE_DOWNLOAD_FAILED: {
				addDialogItem(new TextDialogItem("Failed to download leaderboard data."));
				addDialogItem(new EmptyDialogItem(0.5f));
				addDialogItem(new CommandDialogItem("Retry", [=] {
					tryDownloadingStatisticsForSelected();
				}));
				break;
			}
			case STATE_SUCCESS: {
				//no leaderboard available
				addDialogItem(new TextDialogItem("No leaderboard data available."));
				addDialogItem(new EmptyDialogItem(0.5f));
				addDialogItem(new CommandDialogItem("Retry", [=] {
					tryDownloadingStatisticsForSelected();
				}));
				break;
			}
			default: {
				break;
			}
		}
	}
}

void LeaderboardsDialog::tryDownloadingStatisticsForSelected() {
	unsigned int selected = selectorItem->getSelected();
	if (states[selected] == STATE_DOWNLOAD_REQUESTED || states[selected] == STATE_DOWNLOADING) {
		return;
	}

	SapphireLeaderboards type = supportedTypes[selected];
	states[selected] = STATE_DOWNLOAD_REQUESTED;

	auto* ss = static_cast<SapphireScene*>(getScene());

	ss->getConnection().addStateListenerAndConnect(ss, *this);
	refresh();
}

void LeaderboardsDialog::refresh() {
	if (selectorItem == nullptr) {
		return;
	}

	bool typehighlight = hasHighlighted();
	this->queueRemoveDialogItem(selectorItem);
	this->clearDialogItems();

	addDialogItem(selectorItem);

	putDialogItems(static_cast<SapphireScene*>(getScene()));
	relayout();

	if (typehighlight) {
		setHighlighted(selectorItem);
	}
}

} // namespace userapp
