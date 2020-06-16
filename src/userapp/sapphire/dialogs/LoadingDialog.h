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
 * LoadingDialog.h
 *
 *  Created on: 2017. szept. 15.
 *      Author: sipka
 */

#ifndef JNI_TEST_SAPPHIRE_DIALOGS_LOADINGDIALOG_H_
#define JNI_TEST_SAPPHIRE_DIALOGS_LOADINGDIALOG_H_

#include <sapphire/dialogs/DialogLayer.h>
#include <sapphire/dialogs/items/BusyIndicatorDialogItem.h>
#include <sapphire/dialogs/items/EmptyDialogItem.h>
#include <sapphire/SapphireMenu.h>
#include <sapphire/SapphireScene.h>

namespace userapp {

class LoadingDialog: public DialogLayer, public core::TimeListener {
protected:
	BusyIndicatorDialogItem* busyItem;

	virtual void handleLoadingComplete() = 0;
public:
	LoadingDialog(SapphireUILayer* menu)
			: DialogLayer(menu) {
		getParent()->getScene()->getWindow()->foregroundTimeListeners += *this;
		setTitle("Loading");
		busyItem = new BusyIndicatorDialogItem("Please wait. Loading levels");
		addDialogItem(busyItem);
		addDialogItem(new EmptyDialogItem(0.5f));
		addDialogItem(new CommandDialogItem("Back", [=] {
			core::TimeListener::unsubscribe();
			this->dismiss();
		}));
	}

	virtual void onTimeChanged(const core::time_millis& time, const core::time_millis& previous) override {
		if (static_cast<SapphireScene*>(getParent()->getScene())->isLevelsLoaded()) {
			core::TimeListener::unsubscribe();
			this->dismiss();
			handleLoadingComplete();
		}
	}
};

}  // namespace userapp

#endif /* JNI_TEST_SAPPHIRE_DIALOGS_LOADINGDIALOG_H_ */
