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
 * LinkProgressDialog.cpp
 *
 *  Created on: 2017. jul. 23.
 *      Author: sipka
 */

#include <sapphire/dialogs/LinkProgressDialog.h>
#include <sapphire/SapphireScene.h>
#include <sapphire/dialogs/items/EditTextDialogItem.h>
#include <sapphire/dialogs/ParentDismisserDialogLayer.h>
#include <sapphire/dialogs/items/BusyIndicatorDialogItem.h>

namespace userapp {

LinkProgressDialog::LinkProgressDialog(SapphireUILayer* parent)
		: DialogLayer(parent) {
	infoText = new TextDialogItem("Your game progress will be automatically synchronized between your linked devices.");
	setTitle(LINK_PROGRESS_DIALOG_TITLE);
	setupUI();
}

LinkProgressDialog::~LinkProgressDialog() {
}

void LinkProgressDialog::onConnected(CommunityConnection* connection) {
}

void LinkProgressDialog::onLoggedIn(CommunityConnection* connection) {
	connection->sendLinkIdentifierRequest();
	connection->linkIdentifierRequestEvents += linkIdentifierListener;
}

void LinkProgressDialog::onDisconnected(CommunityConnection* connection) {
	showRetryUI("Connection lost.");
}

void LinkProgressDialog::onConnectionFailed(CommunityConnection* connection) {
	showRetryUI("Connection failed.");
}

void LinkProgressDialog::showRetryUI(const char* message) {
	bool hashighlight = hasHighlighted();
	removeDialogItem(infoText);
	clearDialogItems();

	addDialogItem(infoText);

	addDialogItem(new EmptyDialogItem(0.5f));
	addDialogItem(new TextDialogItem(message));

	addDialogItem(new EmptyDialogItem(0.5f));
	auto* retrycommand = new CommandDialogItem("Retry");
	retrycommand->setHandler([=] {
		bool hhligh = hasHighlighted();
		queueRemoveDialogItem(infoText);
		queueRemoveDialogItem(retrycommand);
		clearDialogItems();
		setupUI();
		this->relayout();
		startConnection();
		if (hashighlight) {
			displayKeyboardSelection();
		}
		delete retrycommand;
	});
	addDialogItem(retrycommand);
	addDialogItem(new CommandDialogItem("Cancel", [=] {
		dismiss();
	}));
	this->relayout();
	if (hashighlight) {
		displayKeyboardSelection();
	}
}
void LinkProgressDialog::startConnection() {
	auto* ss = static_cast<SapphireScene*>(getScene());
	auto& connection = ss->getConnection();
	connection.addStateListenerAndConnect(ss, *this);
}
void LinkProgressDialog::setScene(Scene* scene) {
	DialogLayer::setScene(scene);
	startConnection();
}
void LinkProgressDialog::displayInvalidIdenfitierDialog() {
	auto* info = new DialogLayer(this);
	info->setTitle("Invalid ID");
	info->addDialogItem(new TextDialogItem("Invalid device ID."));
	info->addDialogItem(new EmptyDialogItem(0.5f));
	info->addDialogItem(new CommandDialogItem("Back", [=] {
		info->dismiss();
		setHighlighted(remoteIdEdit);
	}));
	info->show(getScene(), true);
}
void LinkProgressDialog::acceptIdentifier(const char* number, unsigned int numberlen) {
	if (numberlen != SAPPHIRE_LINK_NUMBER_LENGTH) {
		displayInvalidIdenfitierDialog();
	} else {
		uint32 value = stringToNumber(number, numberlen);
		if (value < SAPPHIRE_LINK_NUMBER_MINIMUM || value > SAPPHIRE_LINK_NUMBER_MAXIMUM || value == selfID) {
			//shame on me for using goto
			displayInvalidIdenfitierDialog();
		} else {
			acceptIdentifier(value);
		}
	}
}
void LinkProgressDialog::acceptIdentifier(uint32 id) {
	LOGI()<< "Enter remote identifier: " << id;

	auto& connection = static_cast<SapphireScene*>(getScene())->getConnection();
	linkResultListener = CommunityConnection::LinkResultListener::make_listener([=](SapphireCommError error) {
				LOGI() << "Link result " << error;
				linkResultListener.unsubscribe();
				linkResulted = true;
				DialogLayer* dialog= new ParentDismisserDialogLayer(this);
				switch (error) {
					case SapphireCommError::NoError: {
						dialog->setTitle("Link success");
						dialog->addDialogItem(new TextDialogItem("Your devices has been linked successfully!"));
						break;
					}
					case SapphireCommError::SameHardware: {
						dialog->setTitle("Link failed");
						dialog->addDialogItem(new TextDialogItem("You cannot link the device to itself."));
						break;
					}
					case SapphireCommError::AlreadyLinked: {
						dialog->setTitle("Already linked");
						dialog->addDialogItem(new TextDialogItem("Your progress is already shared between your devices."));
						break;
					}
					default: {
						dialog->setTitle("Link failed");
						dialog->addDialogItem(new TextDialogItem("An unknown error occurred during linking."));
						break;
					}
				}
				dialog->addDialogItem(new EmptyDialogItem(0.5f));
				dialog->addDialogItem(new CommandDialogItem( "Ok", [=] {
									dialog->dismiss();
								}));
				dialog->show(getScene(), true);
			});
	connection.linkResultEvents += linkResultListener;
	connection.sendLinkRemoteIdentifier(id);

	addDialogItemAfter(remoteIdEdit, new BusyIndicatorDialogItem(FixedString {"Awaiting confirmation ("}+ numberToString(id) + ")"));

	queueRemoveDialogItem(remoteIdEdit);
	queueRemoveDialogItem(sendItem);
	finishRemoveDialogItem();
	delete remoteIdEdit;
	delete sendItem;
	remoteIdEdit = nullptr;
	sendItem = nullptr;
}

void LinkProgressDialog::setupUI() {
	addDialogItem(infoText);
	auto* acqempty = new EmptyDialogItem(0.5f);
	addDialogItem(acqempty);

	auto* acquiring = new BusyIndicatorDialogItem("Acquiring ID");
	addDialogItem(acquiring);

	auto* emptyitem = new EmptyDialogItem(0.5f);
	addDialogItem(emptyitem);
	addDialogItem(new CommandDialogItem("Cancel", [=] {
		dismiss();
	}));

	linkIdentifierListener = CommunityConnection::LinkIdentifierRequestListener::make_listener([=](uint32 id) {
		selfID = id;
		queueRemoveDialogItem(acquiring);
		delete acquiring;

		addDialogItemAfter(infoText, new TextDialogItem("Enter the ID of your other device."));
		addDialogItemAfter(infoText, new EmptyDialogItem(0.25f));
		remoteIdEdit = new EditTextDialogItem("Device ID:", "", SAPPHIRE_LINK_NUMBER_LENGTH + 4);
		remoteIdEdit->setContentMaximumLength(SAPPHIRE_LINK_NUMBER_LENGTH);
		remoteIdEdit->setNumericOnly(true);
		remoteIdEdit->setReturnHandler([=] {
					acceptIdentifier(remoteIdEdit->getContent(), remoteIdEdit->getContentLength());
					return true;
				});
		auto* idtext = new TextDialogItem(FixedString {"Your ID: "}+ numberToString(id));
		addDialogItemAfter(acqempty, idtext);
		addDialogItemAfter(idtext, remoteIdEdit);
		sendItem = new CommandDialogItem( "Send",[=] {
					acceptIdentifier(remoteIdEdit->getContent(), remoteIdEdit->getContentLength());
				});
		addDialogItemAfter(emptyitem, sendItem);
		this->relayout();
		if(this->hasHighlighted()) {
			this->setHighlighted(remoteIdEdit);
		}
	});
	invalidate();
}

void LinkProgressDialog::dismiss() {
	DialogLayer::dismiss();
	if (!linkResulted) {
		auto& connection = static_cast<SapphireScene*>(getScene())->getConnection();
		connection.sendLinkCancel();
	}

	StateListener::unsubscribe();
	linkIdentifierListener.unsubscribe();
	if (linkResultListener != nullptr) {
		linkResultListener.unsubscribe();
	}
}

} // namespace userapp
