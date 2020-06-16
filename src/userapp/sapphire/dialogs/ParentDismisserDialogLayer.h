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
 * ParentDismisserDialogLayer.h
 *
 *  Created on: 2016. aug. 13.
 *      Author: sipka
 */

#ifndef TEST_SAPPHIRE_DIALOGS_PARENTDISMISSERDIALOGLAYER_H_
#define TEST_SAPPHIRE_DIALOGS_PARENTDISMISSERDIALOGLAYER_H_

#include <sapphire/dialogs/DialogLayer.h>

namespace userapp {

class ParentDismisserDialogLayer: public DialogLayer {
public:
	using DialogLayer::DialogLayer;

	virtual void dismiss() override {
		DialogLayer::dismiss();
		getParent()->dismiss();
	}
	void dismissKeepParent() {
		DialogLayer::dismiss();
	}
};

}  // namespace userapp

#endif /* TEST_SAPPHIRE_DIALOGS_PARENTDISMISSERDIALOGLAYER_H_ */
