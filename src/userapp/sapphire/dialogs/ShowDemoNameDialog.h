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
 * ShowDemoDialogName.h
 *
 *  Created on: 2016. dec. 23.
 *      Author: sipka
 */

#ifndef TEST_SAPPHIRE_DIALOGS_SHOWDEMONAMEDIALOG_H_
#define TEST_SAPPHIRE_DIALOGS_SHOWDEMONAMEDIALOG_H_

#include <sapphire/dialogs/DialogLayer.h>
#include <sapphire/dialogs/items/EditTextDialogItem.h>
#include <sapphire/sapphireconstants.h>

namespace userapp {

template<typename Functor>
void ShowDemoNameDialog(SapphireUILayer* parent, Functor&& f) {
	DialogLayer* namelayer = new DialogLayer(parent);
	namelayer->setTitle("Save demo");

	auto* nameitem = new EditTextDialogItem("Demo name:", "Demo");
	nameitem->setContentMaximumLength(SAPPHIRE_DEMO_NAME_MAX_LEN);
	nameitem->setReturnHandler([=] {
		namelayer->dismiss();
		f(FixedString {nameitem->getContent(), nameitem->getContentLength()});
		return true;
	});
	namelayer->addDialogItem(nameitem);
	namelayer->addDialogItem(new EmptyDialogItem(0.5f));
	namelayer->addDialogItem(new CommandDialogItem { "Save", [=] {
		namelayer->dismiss();
		f(FixedString {nameitem->getContent(), nameitem->getContentLength()});
	} });
	namelayer->addDialogItem(new CommandDialogItem { "Cancel", [=] {
		namelayer->dismiss();
	} });
	namelayer->show(parent->getScene(), true);
}

}  // namespace userapp

#endif /* TEST_SAPPHIRE_DIALOGS_SHOWDEMONAMEDIALOG_H_ */
