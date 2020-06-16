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
 * LevelDetailsLayer.cpp
 *
 *  Created on: 2016. apr. 24.
 *      Author: sipka
 */

#include <framework/layer/Layer.h>
#include <framework/utils/FixedString.h>
#include <framework/utils/utility.h>
#include <sapphire/LevelEditorLayer.h>
#include <sapphire/dialogs/LevelDetailsLayer.h>
#include <sapphire/LevelSelectorLayer.h>
#include <sapphire/DemoLayer.h>
#include <sapphire/level/Demo.h>
#include <sapphire/level/DemoInfo.h>
#include <sapphire/PlayerLayer.h>
#include <sapphire/SapphireScene.h>
#include <sapphire/SapphireUILayer.h>
#include <sapphire/dialogs/items/LevelRatingDialogItem.h>
#include <sapphire/DemoReplayerLayer.h>
#include <sapphire/DemoRecorderLayer.h>
#include <sapphire/community/CommunityLayer.h>
#include <sapphire/dialogs/items/EditTextDialogItem.h>
#include <sapphire/dialogs/ParentDismisserDialogLayer.h>
#include <sapphire/sapphireconstants.h>
#include <sapphire/dialogs/LeaderboardsDialog.h>
#include <sapphire/dialogs/items/BusyIndicatorDialogItem.h>

using namespace rhfw;

namespace userapp {
class LevelUploadDialogLayer: public DialogLayer, private CommunityConnection::StateListener {
	CommunityConnection::LevelUploadListener::Listener uploadListener;
	virtual void onLoggedIn(CommunityConnection* connection) override {
		CommunityConnection::StateListener::unsubscribe();
		//TODO Change dialog text connecting-uploading?

		connection->levelUploadEvents += uploadListener;
		connection->uploadLevel(static_cast<LevelDetailsLayer*>(getParent())->level);
	}
	virtual void onDisconnected(CommunityConnection* connection) override {
		CommunityConnection::StateListener::unsubscribe();
		if (uploadListener == nullptr) {
			return;
		}
		//failed to upload
		onConnectionFailed(connection);
	}
	virtual void onConnectionFailed(CommunityConnection* connection) override {
		CommunityConnection::StateListener::unsubscribe();
		if (uploadListener == nullptr) {
			return;
		}

		LevelDetailsLayer* parent = static_cast<LevelDetailsLayer*>(getParent());

		dismiss();
		auto* dialog = new DialogLayer(parent);
		dialog->setTitle("Failure");
		dialog->addDialogItem(new TextDialogItem("Failed to establish connection with the community server."));
		dialog->addDialogItem(new EmptyDialogItem(0.5f));
		dialog->addDialogItem(new CommandDialogItem("Retry", [=] {
			dialog->dismiss();

			LevelUploadDialogLayer* uploader = new LevelUploadDialogLayer(parent);
			uploader->show(parent->getScene(), true);
		}));
		dialog->addDialogItem(new CommandDialogItem("Back", [=] {
			dialog->dismiss();
		}));
		dialog->show(getScene(), true);

	}
public:
	LevelUploadDialogLayer(LevelDetailsLayer* parent)
			: DialogLayer(parent) {
		setTitle("Uploading");
		addDialogItem(new BusyIndicatorDialogItem(FixedString { "Uploading level: " } + parent->descriptor->getTitle()));
		addDialogItem(new EmptyDialogItem(0.5f));
		addDialogItem(new CommandDialogItem("Cancel", [=] {
			uploadListener = nullptr;
			dismiss();
		}));

		uploadListener = CommunityConnection::LevelUploadListener::make_listener([=](SapphireCommError error, const SapphireUUID& uuid) {
			if(uuid == parent->descriptor->uuid) {
				dismiss();
				switch (error) {
					case SapphireCommError::LevelDemoRemoved:
					case SapphireCommError::NoError: {
						/*success*/
						parent->removeUploadItems();

						DialogLayer* notif = new DialogLayer(getParent());
						notif->setTitle("Success");
						notif->addDialogItem(new TextDialogItem("Level successfully uploaded!"));
						notif->addDialogItem(new EmptyDialogItem(0.5f));
						notif->addDialogItem(new CommandDialogItem("Okay", [=] {
											notif->dismiss();
										}));
						notif->show(getScene(), true);
						if (parent->selectorLayer != nullptr) {
							parent->selectorLayer->reloadLevels();
						}
						break;
					}
					case SapphireCommError::LevelAlreadyExists: {
						parent->removeUploadItems();

						DialogLayer* notif = new DialogLayer(getParent());
						notif->setTitle("Failure");
						notif->addDialogItem(new TextDialogItem("Level already exists!"));
						notif->addDialogItem(new EmptyDialogItem(0.5f));
						notif->addDialogItem(new CommandDialogItem("Okay", [=] {
											notif->dismiss();
										}));
						notif->show(getScene(), true);
						break;
					}
					case SapphireCommError::LevelNoDemo: {
						DialogLayer* notif = new DialogLayer(getParent());
						notif->setTitle("Failure");
						notif->addDialogItem(new TextDialogItem("Please provide at least one demo with the level!"));
						notif->addDialogItem(new EmptyDialogItem(0.5f));
						notif->addDialogItem(new CommandDialogItem("Okay", [=] {
											notif->dismiss();
										}));
						notif->show(getScene(), true);
						break;
					}
					case SapphireCommError::LevelInvalidDemo: {
						DialogLayer* notif = new DialogLayer(getParent());
						notif->setTitle("Failure");
						notif->addDialogItem(new TextDialogItem("The demo provided doesn't finish properly!"));
						notif->addDialogItem(new EmptyDialogItem(0.5f));
						notif->addDialogItem(new CommandDialogItem("Okay", [=] {
											notif->dismiss();
										}));
						notif->show(getScene(), true);
						break;
					}
					default: {
						/*try again later*/
						DialogLayer* notif = new DialogLayer(getParent());
						notif->setTitle("Failure");
						notif->addDialogItem(new TextDialogItem("Unknown server error."));
						notif->addDialogItem(new EmptyDialogItem(0.5f));
						notif->addDialogItem(new CommandDialogItem("Okay", [=] {
											notif->dismiss();
										}));
						notif->show(getScene(), true);
						break;
					}
				}
				uploadListener = nullptr;
			}
		});
		SapphireScene* ss = static_cast<SapphireScene*>(parent->getScene());
		ss->getConnection().addStateListenerAndConnect(ss, *this);
	}
};

LevelDetailsLayer::LevelDetailsLayer(SapphireUILayer* parent, const SapphireLevelDescriptor* descriptor, Level arglevel)
		: DialogLayer(parent), descriptor(descriptor), level(util::move(arglevel)) {
	setTitle(descriptor->title);
	setColors(descriptor->difficulty);

	auto ss = static_cast<SapphireScene*>(parent->getScene());

	ss->loadCustomDemos(level, descriptor);

	bool hadmeta = false;
	if (level.getInfo().description.length() > 0) {
		addDialogItem(new TextDialogItem { level.getInfo().description });
		hadmeta = true;
	}
	if (level.getInfo().author.getUserName().length() > 0) {
		addDialogItem(new TextDialogItem { FixedString { "Author: " } + level.getInfo().author.getUserName() });
		hadmeta = true;
	}
	if (level.getInfo().category != SapphireLevelCategory::None) {
		addDialogItem(new TextDialogItem(FixedString { "Category: " } + categoryToString(level.getInfo().category)));
		hadmeta = true;
	}
	if (hadmeta) {
		addDialogItem(new EmptyDialogItem(0.5f));
	}

	CommandDialogItem* playitem = new CommandDialogItem { "Play", [=] {
		this->level.setRandomSeed((unsigned int) core::MonotonicTime::getCurrent());
		auto* layer = new PlayerLayer(this->parent, this->descriptor, util::move(level));
		layer->setSelectorLayer(selectorLayer);
		dismiss();
		layer->show(getScene());
	} };

	if (descriptor->hasSuspendedGame) {
		auto* emptysuspended = new EmptyDialogItem(0.25f);
		auto* contsuspended = new CommandDialogItem("Continue suspended game", [=] {
			ss->loadSuspendedLevel(descriptor, level);
			auto* layer = new PlayerLayer(this->parent, this->descriptor, util::move(level));
			layer->setSuspendedLevel();
			layer->setSelectorLayer(selectorLayer);
			dismiss();
			layer->show(getScene());
		});
		auto* delsuspended = new CommandDialogItem("Delete suspended game");
		delsuspended->setHandler([=] {
			ss->deleteSuspendedLevel(descriptor);
			this->queueRemoveDialogItem(emptysuspended);
			this->queueRemoveDialogItem(contsuspended);
			this->queueRemoveDialogItem(delsuspended);
			this->finishRemoveDialogItem();

			if(this->hasHighlighted()) {
				this->setHighlighted(playitem);
			}

			delete emptysuspended;
			delete contsuspended;
			delete delsuspended;
		});
		addDialogItem(contsuspended);
		addDialogItem(delsuspended);
		addDialogItem(emptysuspended);
	}

	addDialogItem(playitem);

	addDialogItem(new EmptyDialogItem(0.25f));
	if (level.getDemoCount() > 0) {
		for (unsigned int i = 0; i < level.getDemoCount(); ++i) {
			auto* demo = level.getDemo(i);
			unsigned int index = i;

			addDialogItem(new CommandDialogItem(FixedString { "View: " } + demo->info.title, [=] {
				auto* layer = new DemoReplayerLayer(this->parent, this->descriptor, util::move(level), index);
				layer->setSelectorLayer(selectorLayer);
				dismiss();
				layer->show(getScene());
			}));
		}
	}
	addDialogItem(new CommandDialogItem("Record new demo", [=] {
		auto* layer = new DemoRecorderLayer(this->parent, this->descriptor, util::move(level));
		layer->setSelectorLayer(selectorLayer);
		dismiss();
		layer->show(getScene());
	}));
	addDialogItem(new EmptyDialogItem(0.25f));
	addDialogItem(new CommandDialogItem("Statistics", [=] {
		ss->showStatisticsDialogForLevel(this, descriptor);
	}));
	if (descriptor->hasLeaderboards()) {
		addDialogItem(new CommandDialogItem("Leaderboards", [=] {
			ss->showLeaderboardsDialogForLevel(this, descriptor);
		}));
	}

	if (descriptor->isEditable()) {
		editempty = new EmptyDialogItem(0.5f);
		edititem = new CommandDialogItem("Edit level", [this, descriptor] {
			LevelEditorLayer* editlayer = new LevelEditorLayer(getParent(), descriptor);
			editlayer->setLevelSelectorLayer(selectorLayer);
			dismiss();
			editlayer->show(getScene());
		});
		addDialogItem(editempty);
		addDialogItem(edititem);
		if (ss->isLevelUploadable(descriptor)) {
			uploadItem = new CommandDialogItem("Upload level", [=] {
				startUpload();
			});
			addDialogItem(uploadItem);
		}
	}

	if (descriptor->isRateable()) {
		addDialogItem(new LevelRatingDialogItem(descriptor));
	}
	if (descriptor->isReportable()) {
		addReportItem();
	}

#if RHFW_DEBUG
	{
		static const char* LEVEL_STATE_LABELS[] { "Unseen", "Unfinished", "Completed" };
		auto* stateitem = new EnumDialogItem(1.0f, "Level state: ", LEVEL_STATE_LABELS, (unsigned int) descriptor->state, 3);
		addDialogItem(stateitem);
		stateitem->setHandler([=] {
			ss->updateLevelState(descriptor, (LevelState) stateitem->getSelected());
			dismiss();
		});

		static const char* LEADERBOARD_LABELS[] { "", "gems", "time", "gems time", "steps", "steps gems", "steps time", "steps time gems" };
		auto* lbitem = new EnumDialogItem(1.0f, "Leaderboards: ", LEADERBOARD_LABELS, (unsigned int) descriptor->leaderboards, 8);
		addDialogItem(lbitem);
		lbitem->setHandler([=] {
			StorageDirectoryDescriptor leaderboardDirectory {StorageDirectoryDescriptor::Root() + "leaderboards"};
			leaderboardDirectory.create();
			StorageFileDescriptor fd {leaderboardDirectory.getPath()+(const char*) descriptor->uuid.asString()};
			const char* data = LEADERBOARD_LABELS[lbitem->getSelected()];
			fd.openOutputStream().write(data, strlen(data));
			dismiss();
		});
		addDialogItem(new CommandDialogItem("Copy level", [=] {
			LevelEditorLayer* editlayer = new LevelEditorLayer(getParent(), util::move(level));
			editlayer->setLevelSelectorLayer(selectorLayer);
			dismiss();
			editlayer->show(getScene());
		}));
		addDialogItem(new CommandDialogItem("Check demos", [=] {
			ss->checkLevelDemo(descriptor);
		}));
	}
#endif /* RHFW_DEBUG */
}
LevelDetailsLayer::~LevelDetailsLayer() {
}

void LevelDetailsLayer::startUpload() {
	auto* ss = static_cast<SapphireScene*>(getScene());

	if (level.getDemoCount() == 0) {
		DialogLayer* info = new DialogLayer(this);
		info->setTitle("No demos found");
		info->addDialogItem(new TextDialogItem("To upload a level, please provide at least one working demo."));
		info->addDialogItem(new EmptyDialogItem(0.5f));
		info->addDialogItem(new CommandDialogItem("Record new demo", [=] {
			auto* layer = new DemoRecorderLayer(this->getParent(), this->descriptor, util::move(level));
			layer->setSelectorLayer(selectorLayer);
			info->dismiss();
			dismiss();
			layer->show(getScene());
		}));
		info->showDialog(getScene());
		return;
	}

	{
		DialogLayer* confirm = new DialogLayer(this);
		confirm->setTitle("Upload level");
		confirm->addDialogItem(new TextDialogItem("Are sure you want to upload your level?"));
		confirm->addDialogItem(new EmptyDialogItem(0.5f));
		confirm->addDialogItem(new CommandDialogItem("No", [=] {
			confirm->dismiss();
		}));
		confirm->addDialogItem(new CommandDialogItem("Yes", [=] {
			confirm->dismiss();
			auto* uploader = new LevelUploadDialogLayer(this);
			uploader->show(ss, true);
		}));
		confirm->show(ss, true);
	}
}

void LevelDetailsLayer::removeUploadItems() {
	queueRemoveDialogItem(uploadItem);
	queueRemoveDialogItem(edititem);
	queueRemoveDialogItem(editempty);
	finishRemoveDialogItem();
	delete edititem;
	delete editempty;
	delete uploadItem;
}

void LevelDetailsLayer::addReportItem() {
	addDialogItem(new CommandDialogItem("Report level", [=] {
		auto ss = static_cast<SapphireScene*>(getParent()->getScene());
		DialogLayer* info = new DialogLayer(this);
		EditTextDialogItem* reasonitem = new EditTextDialogItem("Reason:", nullptr);
		reasonitem->setContentMaximumLength(SAPPHIRE_FEEDBACK_MAX_LEN);
		info->setTitle("Report level");
		info->addDialogItem(new TextDialogItem("By reporting a level you can help the moderators handle unappropriate content."));
		info->addDialogItem(new EmptyDialogItem(0.5f));
		info->addDialogItem(reasonitem);

		info->addDialogItem(new EmptyDialogItem(0.5f));
		info->addDialogItem(new CommandDialogItem( "Submit", [=] {
							if (reasonitem->getContentLength() == 0) {
								DialogLayer* error = new DialogLayer(info);
								error->setTitle("Reason missing");
								error->addDialogItem(new TextDialogItem("Please specify a reason for your report!"));
								error->addDialogItem(new EmptyDialogItem(0.5f));
								error->addDialogItem(new CommandDialogItem( "Back", [=] {
													error->dismiss();
													info->setHighlighted(reasonitem);
												}));
								error->show(ss, true);
							} else {
								DialogLayer* error;
								if(ss->getConnection().reportLevel(descriptor->uuid, reasonitem->getContentString())) {
									error = new ParentDismisserDialogLayer(info);
									error->setTitle("Report submitted");
									error->addDialogItem(new TextDialogItem("Thank you for your help!"));
									error->addDialogItem(new EmptyDialogItem(0.5f));
									error->addDialogItem(new CommandDialogItem( "Back", [=] {
														error->dismiss();
													}));
								} else {
									error = new DialogLayer(info);
									error->setTitle("Failure");
									error->addDialogItem(new TextDialogItem("Failed to submit report, please try again later."));
									error->addDialogItem(new EmptyDialogItem(0.5f));
									error->addDialogItem(new CommandDialogItem( "Back", [=] {
														error->dismiss();
													}));
								}
								error->show(ss, true);
							}
						}));

		info->show(ss, true);
	}));
}

}  // namespace userapp

