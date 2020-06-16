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
 * EmptyDialogItem.h
 *
 *  Created on: 2016. aug. 9.
 *      Author: sipka
 */

#ifndef TEST_SAPPHIRE_DIALOGS_ITEMS_EMPTYDIALOGITEM_H_
#define TEST_SAPPHIRE_DIALOGS_ITEMS_EMPTYDIALOGITEM_H_

#include <sapphire/dialogs/DialogLayer.h>

namespace userapp {

class EmptyDialogItem: public DialogItem {
public:
	EmptyDialogItem(float weight)
			: DialogItem { weight } {
	}
	virtual float measureTextWidth(float textsize, float maxwidth) override {
		return 0.0f;
	}
	virtual float measureTextHeight(float textsize, float maxwidth) override {
		return minLines;
	}

	virtual void draw(const rhfw::Matrix2D& mvp) override {
	}
};

}  // namespace userapp

#endif /* TEST_SAPPHIRE_DIALOGS_ITEMS_EMPTYDIALOGITEM_H_ */
