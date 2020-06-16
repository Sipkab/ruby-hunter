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
 * BusyIndicatorDialogItem.cpp
 *
 *  Created on: 2017. aug. 4.
 *      Author: sipka
 */

#include <sapphire/dialogs/items/BusyIndicatorDialogItem.h>

#define DOT_INVALIDATE_MS 400

namespace userapp {
using namespace rhfw;

BusyIndicatorDialogItem::BusyIndicatorDialogItem(FixedString name)
		: DialogItem(1.0f), name(util::move(name)) {
}

BusyIndicatorDialogItem::~BusyIndicatorDialogItem() {
}

void BusyIndicatorDialogItem::draw(const rhfw::Matrix2D& mvp) {
	static_assert(MAX_DOTS == 3, "MAX_DOTS not equals 3");

	dots = (((long long) (core::time_millis) core::MonotonicTime::getCurrent()) / DOT_INVALIDATE_MS) % (MAX_DOTS + 1);

	Vector2F pos { (getRect().leftTop() + getRect().leftBottom()) / 2 };
	float w = drawString(mvp, name, dialog->getFont(), dialog->getUiColor(), pos, dialog->getSharedTextSize(),
			Gravity::CENTER_VERTICAL | Gravity::LEFT);
	pos.x() += w;

	char buff[MAX_DOTS + 1] { "..." };
	drawString(mvp, buff, buff + dots, dialog->getFont(), dialog->getUiColor(), pos, dialog->getSharedTextSize(),
			Gravity::CENTER_VERTICAL | Gravity::LEFT);
}

float BusyIndicatorDialogItem::measureTextWidth(float textsize, float maxwidth) {
	return dialog->getFont()->measureText(name, textsize) + dialog->getFont()->measureText("...", textsize);
}

bool BusyIndicatorDialogItem::shouldInvalidate() {
	return (((long long) (core::time_millis) core::MonotonicTime::getCurrent()) / DOT_INVALIDATE_MS) % (MAX_DOTS + 1) != this->dots;
}

} // namespace userapp
