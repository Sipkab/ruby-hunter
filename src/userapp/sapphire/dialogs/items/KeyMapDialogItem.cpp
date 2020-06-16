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
#include <sapphire/dialogs/items/KeyMapDialogItem.h>
#include <framework/io/key/KeyEvent.h>
#include <framework/utils/utility.h>

namespace userapp {
using namespace rhfw;

class KeyCodeStringMap {
	const char* stringMap[(unsigned int) KeyCode::_count_of_entries];
public:
	KeyCodeStringMap() {
		for (int i = 0; i < (unsigned int) KeyCode::_count_of_entries; ++i) {
			stringMap[i] = "";
		}
		stringMap[(unsigned int) KeyCode::KEY_A] = "A";
		stringMap[(unsigned int) KeyCode::KEY_B] = "B";
		stringMap[(unsigned int) KeyCode::KEY_C] = "C";
		stringMap[(unsigned int) KeyCode::KEY_D] = "D";
		stringMap[(unsigned int) KeyCode::KEY_E] = "E";
		stringMap[(unsigned int) KeyCode::KEY_F] = "F";
		stringMap[(unsigned int) KeyCode::KEY_G] = "G";
		stringMap[(unsigned int) KeyCode::KEY_H] = "H";
		stringMap[(unsigned int) KeyCode::KEY_I] = "I";
		stringMap[(unsigned int) KeyCode::KEY_J] = "J";
		stringMap[(unsigned int) KeyCode::KEY_K] = "K";
		stringMap[(unsigned int) KeyCode::KEY_L] = "L";
		stringMap[(unsigned int) KeyCode::KEY_M] = "M";
		stringMap[(unsigned int) KeyCode::KEY_N] = "N";
		stringMap[(unsigned int) KeyCode::KEY_O] = "O";
		stringMap[(unsigned int) KeyCode::KEY_P] = "P";
		stringMap[(unsigned int) KeyCode::KEY_Q] = "Q";
		stringMap[(unsigned int) KeyCode::KEY_R] = "R";
		stringMap[(unsigned int) KeyCode::KEY_S] = "S";
		stringMap[(unsigned int) KeyCode::KEY_T] = "T";
		stringMap[(unsigned int) KeyCode::KEY_U] = "U";
		stringMap[(unsigned int) KeyCode::KEY_V] = "V";
		stringMap[(unsigned int) KeyCode::KEY_W] = "W";
		stringMap[(unsigned int) KeyCode::KEY_X] = "X";
		stringMap[(unsigned int) KeyCode::KEY_Y] = "Y";
		stringMap[(unsigned int) KeyCode::KEY_Z] = "Z";

		stringMap[(unsigned int) KeyCode::KEY_0] = "0";
		stringMap[(unsigned int) KeyCode::KEY_1] = "1";
		stringMap[(unsigned int) KeyCode::KEY_2] = "2";
		stringMap[(unsigned int) KeyCode::KEY_3] = "3";
		stringMap[(unsigned int) KeyCode::KEY_4] = "4";
		stringMap[(unsigned int) KeyCode::KEY_5] = "5";
		stringMap[(unsigned int) KeyCode::KEY_6] = "6";
		stringMap[(unsigned int) KeyCode::KEY_7] = "7";
		stringMap[(unsigned int) KeyCode::KEY_8] = "8";
		stringMap[(unsigned int) KeyCode::KEY_9] = "9";

		stringMap[(unsigned int) KeyCode::KEY_F1] = "F1";
		stringMap[(unsigned int) KeyCode::KEY_F2] = "F2";
		stringMap[(unsigned int) KeyCode::KEY_F3] = "F3";
		stringMap[(unsigned int) KeyCode::KEY_F4] = "F4";
		stringMap[(unsigned int) KeyCode::KEY_F5] = "F5";
		stringMap[(unsigned int) KeyCode::KEY_F6] = "F6";
		stringMap[(unsigned int) KeyCode::KEY_F7] = "F7";
		stringMap[(unsigned int) KeyCode::KEY_F8] = "F8";
		stringMap[(unsigned int) KeyCode::KEY_F9] = "F9";
		stringMap[(unsigned int) KeyCode::KEY_F10] = "F10";
		stringMap[(unsigned int) KeyCode::KEY_F11] = "F11";
		stringMap[(unsigned int) KeyCode::KEY_F12] = "F12";

		stringMap[(unsigned int) KeyCode::KEY_DIR_UP] = "Up";
		stringMap[(unsigned int) KeyCode::KEY_DIR_DOWN] = "Down";
		stringMap[(unsigned int) KeyCode::KEY_DIR_LEFT] = "Left";
		stringMap[(unsigned int) KeyCode::KEY_DIR_RIGHT] = "Right";

		stringMap[(unsigned int) KeyCode::KEY_SPACE] = "Space";
		stringMap[(unsigned int) KeyCode::KEY_TAB] = "Tab";

		stringMap[(unsigned int) KeyCode::KEY_ENTER] = "Enter";
		stringMap[(unsigned int) KeyCode::KEY_BACKSPACE] = "Backspace";
		stringMap[(unsigned int) KeyCode::KEY_DELETE] = "Delete";

		stringMap[(unsigned int) KeyCode::KEY_HOME] = "Home";
		stringMap[(unsigned int) KeyCode::KEY_END] = "End";
		stringMap[(unsigned int) KeyCode::KEY_ESC] = "Esc";
		stringMap[(unsigned int) KeyCode::KEY_INSERT] = "Insert";
		stringMap[(unsigned int) KeyCode::KEY_PAGE_UP] = "Page Up";
		stringMap[(unsigned int) KeyCode::KEY_PAGE_DOWN] = "Page Down";

		stringMap[(unsigned int) KeyCode::KEY_NUM_0] = "Num 0";
		stringMap[(unsigned int) KeyCode::KEY_NUM_1] = "Num 1";
		stringMap[(unsigned int) KeyCode::KEY_NUM_2] = "Num 2";
		stringMap[(unsigned int) KeyCode::KEY_NUM_3] = "Num 3";
		stringMap[(unsigned int) KeyCode::KEY_NUM_4] = "Num 4";
		stringMap[(unsigned int) KeyCode::KEY_NUM_5] = "Num 5";
		stringMap[(unsigned int) KeyCode::KEY_NUM_6] = "Num 6";
		stringMap[(unsigned int) KeyCode::KEY_NUM_7] = "Num 7";
		stringMap[(unsigned int) KeyCode::KEY_NUM_8] = "Num 8";
		stringMap[(unsigned int) KeyCode::KEY_NUM_9] = "Num 9";
		stringMap[(unsigned int) KeyCode::KEY_NUM_DIV] = "Num /";
		stringMap[(unsigned int) KeyCode::KEY_NUM_MULT] = "Num *";
		stringMap[(unsigned int) KeyCode::KEY_NUM_SUBTRACT] = "Num -";
		stringMap[(unsigned int) KeyCode::KEY_NUM_ADD] = "Num +";
		stringMap[(unsigned int) KeyCode::KEY_NUM_ENTER] = "Num Enter";
		stringMap[(unsigned int) KeyCode::KEY_NUM_DOT] = "Num .";
		stringMap[(unsigned int) KeyCode::KEY_CLEAR] = "Clear";

		stringMap[(unsigned int) KeyCode::KEY_NUMLOCK] = "Num Lock";
		stringMap[(unsigned int) KeyCode::KEY_SCROLLLOCK] = "Scroll Lock";
		stringMap[(unsigned int) KeyCode::KEY_CAPSLOCK] = "Caps Lock";
		stringMap[(unsigned int) KeyCode::KEY_PRINTSCREEN] = "Print Screen";
		stringMap[(unsigned int) KeyCode::KEY_PAUSEBREAK] = "Pause Break";

		stringMap[(unsigned int) KeyCode::KEY_COMMAND] = "Command";
		stringMap[(unsigned int) KeyCode::KEY_CTRL] = "Ctrl";
		stringMap[(unsigned int) KeyCode::KEY_ALT] = "Alt";
		stringMap[(unsigned int) KeyCode::KEY_SHIFT] = "Shift";

		stringMap[(unsigned int) KeyCode::KEY_LEFT_CTRL] = "Left Ctrl";
		stringMap[(unsigned int) KeyCode::KEY_RIGHT_CTRL] = "Right Ctrl";
		stringMap[(unsigned int) KeyCode::KEY_LEFT_ALT] = "Left Alt";
		stringMap[(unsigned int) KeyCode::KEY_RIGHT_ALT] = "Right Alt";
		stringMap[(unsigned int) KeyCode::KEY_LEFT_SHIFT] = "Left Shift";
		stringMap[(unsigned int) KeyCode::KEY_RIGHT_SHIFT] = "Right Shift";

		stringMap[(unsigned int) KeyCode::KEY_GAMEPAD_DPAD_LEFT] = "GP Dpad Left";
		stringMap[(unsigned int) KeyCode::KEY_GAMEPAD_DPAD_UP] = "GP Dpad Up";
		stringMap[(unsigned int) KeyCode::KEY_GAMEPAD_DPAD_RIGHT] = "GP Dpad Right";
		stringMap[(unsigned int) KeyCode::KEY_GAMEPAD_DPAD_DOWN] = "GP Dpad Down";
		stringMap[(unsigned int) KeyCode::KEY_GAMEPAD_START] = "GP Start";
		stringMap[(unsigned int) KeyCode::KEY_GAMEPAD_BACK] = "GP Back";
		stringMap[(unsigned int) KeyCode::KEY_GAMEPAD_LEFT_THUMB] = "GP Left Thumb";
		stringMap[(unsigned int) KeyCode::KEY_GAMEPAD_LEFT_THUMB_LEFT] = "GP LThumb Left";
		stringMap[(unsigned int) KeyCode::KEY_GAMEPAD_LEFT_THUMB_UP] = "GP LThumb Up";
		stringMap[(unsigned int) KeyCode::KEY_GAMEPAD_LEFT_THUMB_RIGHT] = "GP LThumb Right";
		stringMap[(unsigned int) KeyCode::KEY_GAMEPAD_LEFT_THUMB_DOWN] = "GP LThumb Down";
		stringMap[(unsigned int) KeyCode::KEY_GAMEPAD_RIGHT_THUMB] = "GP Right Thumb";
		stringMap[(unsigned int) KeyCode::KEY_GAMEPAD_RIGHT_THUMB_LEFT] = "GP RThumb Left";
		stringMap[(unsigned int) KeyCode::KEY_GAMEPAD_RIGHT_THUMB_UP] = "GP RThumb Up";
		stringMap[(unsigned int) KeyCode::KEY_GAMEPAD_RIGHT_THUMB_RIGHT] = "GP RThumb Right";
		stringMap[(unsigned int) KeyCode::KEY_GAMEPAD_RIGHT_THUMB_DOWN] = "GP RThumb Down";
		stringMap[(unsigned int) KeyCode::KEY_GAMEPAD_LEFT_SHOULDER] = "GP LBack";
		stringMap[(unsigned int) KeyCode::KEY_GAMEPAD_RIGHT_SHOULDER] = "GP RBack";
		stringMap[(unsigned int) KeyCode::KEY_GAMEPAD_LEFT_TRIGGER] = "GP LTrigger";
		stringMap[(unsigned int) KeyCode::KEY_GAMEPAD_RIGHT_TRIGGER] = "GP RTrigger";
		stringMap[(unsigned int) KeyCode::KEY_GAMEPAD_A] = "GP A";
		stringMap[(unsigned int) KeyCode::KEY_GAMEPAD_B] = "GP B";
		stringMap[(unsigned int) KeyCode::KEY_GAMEPAD_X] = "GP X";
		stringMap[(unsigned int) KeyCode::KEY_GAMEPAD_Y] = "GP Y";
	}
	const char* operator[](KeyCode kc) {
		return stringMap[(unsigned int) kc];
	}
};
static KeyCodeStringMap KeyStringMap;

static const char* KeyCodeToString(KeyCode kc) {
	return KeyStringMap[kc];
}

KeyMapDialogItem::KeyMapDialogItem(FixedString name, KeyCode keycode, InputDevice inputdevice)
		: DialogItem(1.0f), name(util::move(name)), keyCode(keycode), keyName(KeyCodeToString(keycode)), inputDevice(inputdevice) {
	setHighlightable(true);
}
#define RETURN_ON_NOT_DOWN()  if (KeyEvent::instance.getAction() != KeyAction::DOWN) return true
bool KeyMapDialogItem::onKeyEvent() {
	auto keycode = KeyEvent::instance.getKeycode();
	if (activated && dialog->isHighlighted(this)) {
		RETURN_ON_NOT_DOWN();
		if(KeyEvent::instance.getInputDevice() != inputDevice) {
			goto general_key_switch;
		}
		switch (keycode) {
			case KeyCode::KEY_RIGHT_SHIFT:
			case KeyCode::KEY_LEFT_SHIFT: {
				keycode = KeyCode::KEY_SHIFT;
				break;
			}
			case KeyCode::KEY_RIGHT_CTRL:
			case KeyCode::KEY_LEFT_CTRL: {
				keycode = KeyCode::KEY_CTRL;
				break;
			}
			case KeyCode::KEY_RIGHT_ALT:
			case KeyCode::KEY_LEFT_ALT: {
				keycode = KeyCode::KEY_ALT;
				break;
			}
			default: {
				break;
			}
		}
		RETURN_ON_NOT_DOWN();
		this->keyCode = keycode;
		this->keyName = KeyCodeToString(keycode);
		activated = false;
		dialog->invalidate();
		return true;
	}

	general_key_switch:

	switch (keycode) {
		case KeyCode::KEY_ENTER: {
			RETURN_ON_NOT_DOWN();
			activated = !activated;
			dialog->invalidate();
			return true;
		}
		case KeyCode::KEY_GAMEPAD_X: {
			//only use X as delete, if gamepad input is recording
			if (inputDevice != InputDevice::GAMEPAD) {
				break;
			}
			/*fall-through*/
		}
		case KeyCode::KEY_DELETE: {
			RETURN_ON_NOT_DOWN();
			this->keyCode = KeyCode::KEY_UNKNOWN;
			this->keyName = "";
			dialog->invalidate();
			return true;
		}
		default: {
			break;
		}
	}
	return false;
}

void KeyMapDialogItem::draw(const rhfw::Matrix2D& mvp) {
	const bool highlighted = dialog->isHighlighted(this);
	Color color = highlighted ? dialog->getUiSelectedColor() : dialog->getUiColor();

	Vector2F pos { (getRect().leftTop() + getRect().leftBottom()) / 2 };
	drawString(mvp, name, dialog->getFont(), color, pos, dialog->getSharedTextSize(), Gravity::LEFT | Gravity::CENTER_VERTICAL);

	Vector2F contentpos { (getRect().rightTop() + getRect().rightBottom()) / 2 };
	contentpos.x() -= dialog->getSharedTextSize() / 8.0f;
	if (activated && dialog->isHighlighted(this)) {
		drawRectangleColor(mvp, color, Rectangle { contentpos.x(), getRect().top, contentpos.x() + dialog->getSharedTextSize() / 8.0f,
				getRect().bottom });
	} else {
		drawString(mvp, keyName, dialog->getFont(), color, contentpos, dialog->getSharedTextSize(),
				Gravity::RIGHT | Gravity::CENTER_VERTICAL);
	}
}
float KeyMapDialogItem::measureTextWidth(float textsize, float maxwidth) {
	return dialog->getFont()->measureText(name.begin(), name.end(), textsize) + textsize * 8.0f;
}
float KeyMapDialogItem::measureTextHeight(float textsize, float maxwidth) {
	return minLines;
}
void KeyMapDialogItem::onHighlightRevoked() {
	activated = false;
}
void KeyMapDialogItem::onSelected(const Vector2F* pointer) {
	activated = !activated;
	if (activated) {
		dialog->setHighlighted(this);
	} else {
		dialog->invalidateHighlight();
	}
}
} // namespace userapp

