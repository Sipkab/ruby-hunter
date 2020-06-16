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
 * LinkProgressDialog.h
 *
 *  Created on: 2017. jul. 23.
 *      Author: sipka
 */

#ifndef JNI_TEST_SAPPHIRE_DIALOGS_LINKPROGRESSDIALOG_H_
#define JNI_TEST_SAPPHIRE_DIALOGS_LINKPROGRESSDIALOG_H_

#include <sapphire/community/CommunityConnection.h>
#include <sapphire/dialogs/DialogLayer.h>

namespace userapp {
using namespace rhfw;
class EditTextDialogItem;
#define LINK_PROGRESS_DIALOG_TITLE "Link devices"

class LinkProgressDialog: public DialogLayer, private CommunityConnection::StateListener {
	virtual void onConnected(CommunityConnection* connection) override;
	virtual void onLoggedIn(CommunityConnection* connection) override;
	virtual void onDisconnected(CommunityConnection* connection) override;
	virtual void onConnectionFailed(CommunityConnection* connection) override;

	DialogItem* textItem = nullptr;
	EditTextDialogItem* remoteIdEdit = nullptr;
	CommandDialogItem* sendItem = nullptr;
	TextDialogItem* infoText = nullptr;

	bool linkResulted = false;

	CommunityConnection::LinkIdentifierRequestListener::Listener linkIdentifierListener;
	CommunityConnection::LinkResultListener::Listener linkResultListener;

	uint32 selfID = 0;

	void acceptIdentifier(uint32 id);
	void acceptIdentifier(const char* number, unsigned int numberlen);

	void displayInvalidIdenfitierDialog();

	void setupUI();
	void showRetryUI(const char* message);

	void startConnection();
public:
	LinkProgressDialog(SapphireUILayer* parent);
	~LinkProgressDialog();

	virtual void setScene(Scene* scene) override;

	virtual void dismiss() override;
};

} // namespace userapp

#endif /* JNI_TEST_SAPPHIRE_DIALOGS_LINKPROGRESSDIALOG_H_ */
