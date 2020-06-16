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
 * EnumDialogItem.cpp
 *
 *  Created on: 2016. aug. 9.
 *      Author: sipka
 */

#include <sapphire/dialogs/items/EnumDialogItem.h>
#include <framework/io/key/KeyEvent.h>

namespace userapp {
#define BREAK_ON_NOT_DOWN()  if (KeyEvent::instance.getAction() != KeyAction::DOWN) break
bool EnumDialogItem::onKeyEvent() {
	switch (KeyEvent::instance.getKeycode()) {
		case KeyCode::KEY_GAMEPAD_DPAD_LEFT:
		case KeyCode::KEY_DIR_LEFT: {
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

}
 // namespace userapp

