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
 * SelectorEnumDialogItem.cpp
 *
 *  Created on: 2017. aug. 3.
 *      Author: sipka
 */

#include <sapphire/dialogs/items/SelectorEnumDialogItem.h>
#include <framework/io/key/KeyEvent.h>

namespace userapp {
using namespace rhfw;

SelectorEnumDialogItem::SelectorEnumDialogItem(const char* const * items, unsigned int selected, unsigned int count)
		: DialogItem(1.0f), items(items), selected(selected), count(count) {
	ASSERT(selected < count);
	setHighlightable(true);
	noHighlightRectDraw = true;
}

SelectorEnumDialogItem::~SelectorEnumDialogItem() {
}

void SelectorEnumDialogItem::draw(const rhfw::Matrix2D& mvp) {
	auto&& rect = getRect();
	float width = this->dialog->getSize().width();
	bool highlighted = dialog->isHighlighted(this);
	for (unsigned int i = 0; i < count; ++i) {
		Rectangle itemrect { max(dialog->getEdgeSize().width(), width / count * i), rect.top, 0, rect.bottom };
		itemrect.right = min(width - dialog->getEdgeSize().width(), itemrect.left + width / count);
		bool fullrect = false;
		if (highlighted) {
			if ((touchHighlightIndex < 0 && i == selected) || (touchHighlightIndex >= 0 && i == touchHighlightIndex)) {
				drawRectangleColor(mvp, dialog->getUiColor(), itemrect);
				fullrect = true;
			}
		}
		if (!fullrect && i == selected) {
			//draw underline
			Rectangle showrect { itemrect.left, itemrect.bottom - rect.height() * 0.1f, itemrect.right, itemrect.bottom };
			if (count == 1) {
				showrect = showrect.inset(Vector2F { width * 0.15f, 0 });
			}
			drawRectangleColor(mvp, dialog->getUiColor(), showrect);
		}
		auto&& textcolor = fullrect ? dialog->getUiSelectedColor() : dialog->getUiColor();
		drawString(mvp, items[i], dialog->getFont(), textcolor, itemrect.middle(), dialog->getSharedTextSize(), Gravity::CENTER);
	}
}

float SelectorEnumDialogItem::measureTextWidth(float textsize, float maxwidth) {
	ASSERT(count > 0);
	float longest = dialog->getFont()->measureText(items[0], textsize);
	for (unsigned int i = 1; i < count; ++i) {
		float len = dialog->getFont()->measureText(items[i], textsize);
		if (len > longest) {
			longest = len;
		}
	}
	return count * (textsize + longest);
}

#define BREAK_ON_NOT_DOWN()  if (KeyEvent::instance.getAction() != KeyAction::DOWN) break
bool SelectorEnumDialogItem::onKeyEvent() {
	switch (KeyEvent::instance.getKeycode()) {
		case KeyCode::KEY_GAMEPAD_LEFT_SHOULDER:
		case KeyCode::KEY_GAMEPAD_DPAD_LEFT:
		case KeyCode::KEY_DIR_LEFT: {
			decrease_selected:

			BREAK_ON_NOT_DOWN();
			--this->selected;
			if(this->selected >= count) {
				this->selected = count - 1;
			}
			if(selectionListener != nullptr) {
				selectionListener->onSelectionChanged(this->selected);
			}
			dialog->invalidate();
			break;
		}
		case KeyCode::KEY_TAB: {
			if ((KeyEvent::instance.getModifiers() & KeyModifiers::SHIFT_ON_BOOL_MASK) != 0) {
				goto decrease_selected;
			}
			// fall-through
		}
		case KeyCode::KEY_GAMEPAD_RIGHT_SHOULDER:
		case KeyCode::KEY_GAMEPAD_DPAD_RIGHT:
		case KeyCode::KEY_DIR_RIGHT: {
			BREAK_ON_NOT_DOWN();
			++this->selected;
			if(this->selected >= count) {
				this->selected = 0;
			}
			if(selectionListener != nullptr) {
				selectionListener->onSelectionChanged(this->selected);
			}
			dialog->invalidate();
			break;
		}
		default: {
			return false;
		}
	}
	return true;
}

int SelectorEnumDialogItem::getIndexForPointer(const Vector2F& pointer) {
	auto&& rect = getRect();
	float width = this->dialog->getSize().width();
	for (unsigned int i = 0; i < count; ++i) {
		Rectangle itemrect { width / count * i, rect.top, 0, rect.bottom };
		itemrect.right = itemrect.left + width / count;
		if (itemrect.isInside(pointer)) {
			return i;
		}
	}
	return -1;
}

void SelectorEnumDialogItem::onHighlighted(Vector2F* pointer) {
	if (pointer == nullptr) {
		touchHighlightIndex = -1;
	} else {
		touchHighlightIndex = getIndexForPointer(*pointer);
	}
}

void SelectorEnumDialogItem::onSelected(const Vector2F* pointer) {
	dialog->invalidate();
	auto starttouch = touchHighlightIndex;
	touchHighlightIndex = -1;
	if (pointer != nullptr) {
		int index = getIndexForPointer(*pointer);
		if (index >= 0) {
			if (index != starttouch) {
				//we released the pointer not on the same as we started
				return;
			}
			if (index != selected) {
				selected = index;
				if (selectionListener != nullptr) {
					selectionListener->onSelectionChanged(this->selected);
				}
			}
			return;
		}
	}
	++this->selected;
	if (this->selected >= count) {
		this->selected = 0;
	}
	if (selectionListener != nullptr) {
		selectionListener->onSelectionChanged(this->selected);
	}
}

}
// namespace userapp
