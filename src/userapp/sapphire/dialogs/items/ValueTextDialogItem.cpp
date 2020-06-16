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
 * ValueTextDialogItem.cpp
 *
 *  Created on: 2017. aug. 3.
 *      Author: sipka
 */

#include <sapphire/dialogs/items/ValueTextDialogItem.h>

namespace userapp {

ValueTextDialogItem::ValueTextDialogItem(FixedString name, FixedString value)
		: DialogItem(1.0f), name(util::move(name)), value(util::move(value)) {
}

ValueTextDialogItem::~ValueTextDialogItem() {
}

void ValueTextDialogItem::draw(const rhfw::Matrix2D& mvp) {
	drawString(mvp, name, dialog->getFont(), dialog->getUiColor(), Vector2F { (getRect().leftTop() + getRect().leftBottom()) / 2 },
			dialog->getSharedTextSize(), Gravity::CENTER_VERTICAL | Gravity::LEFT);
	drawString(mvp, value, dialog->getFont(), dialog->getUiColor(), Vector2F { (getRect().rightTop() + getRect().rightBottom()) / 2 },
			dialog->getSharedTextSize(), Gravity::CENTER_VERTICAL | Gravity::RIGHT);
}

float ValueTextDialogItem::measureTextWidth(float textsize, float maxwidth) {
	return dialog->getFont()->measureText(name, textsize) + textsize * 2 + dialog->getFont()->measureText(value, textsize);
}

} // namespace userapp
