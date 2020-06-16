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
 * ValueTextDialogItem.h
 *
 *  Created on: 2017. aug. 3.
 *      Author: sipka
 */

#ifndef JNI_TEST_SAPPHIRE_DIALOGS_ITEMS_VALUETEXTDIALOGITEM_H_
#define JNI_TEST_SAPPHIRE_DIALOGS_ITEMS_VALUETEXTDIALOGITEM_H_

#include <sapphire/dialogs/DialogLayer.h>

namespace userapp {
using namespace rhfw;

class ValueTextDialogItem: public DialogItem {
	FixedString name;
	FixedString value;
public:
	ValueTextDialogItem(FixedString name, FixedString value);
	~ValueTextDialogItem();

	virtual void draw(const rhfw::Matrix2D& mvp) override;
	virtual float measureTextWidth(float textsize, float maxwidth) override;

	virtual float measureTextHeight(float textsize, float maxwidth) override {
		return minLines;
	}
};

} // namespace userapp

#endif /* JNI_TEST_SAPPHIRE_DIALOGS_ITEMS_VALUETEXTDIALOGITEM_H_ */
