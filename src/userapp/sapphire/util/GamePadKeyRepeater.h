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
 * GamePadKeyRepeater.h
 *
 *  Created on: 2017. aug. 22.
 *      Author: sipka
 */

#ifndef JNI_TEST_UTIL_GAMEPADKEYREPEATER_H_
#define JNI_TEST_UTIL_GAMEPADKEYREPEATER_H_

#include <framework/core/timing.h>
#include <framework/core/Window.h>
#include <gen/types.h>

namespace rhfw {
class GamePadState;
class GamePad;
} // namespace rhfw

namespace userapp {
using namespace rhfw;

template<KeyCode keycode>
class GamePadKeyRepeaterButtonIndexer;

#define MAKE_INDEX(kc, i) template<> class GamePadKeyRepeaterButtonIndexer<KeyCode::kc>{ public: static const unsigned int INDEX = i; }

MAKE_INDEX(KEY_GAMEPAD_DPAD_LEFT, 0);

MAKE_INDEX(KEY_GAMEPAD_DPAD_UP, 1);

MAKE_INDEX(KEY_GAMEPAD_DPAD_RIGHT, 2);

MAKE_INDEX(KEY_GAMEPAD_DPAD_DOWN, 3);

MAKE_INDEX(KEY_GAMEPAD_START, 4);

MAKE_INDEX(KEY_GAMEPAD_BACK, 5);

MAKE_INDEX(KEY_GAMEPAD_LEFT_THUMB, 6);

MAKE_INDEX(KEY_GAMEPAD_LEFT_THUMB_LEFT, 7);

MAKE_INDEX(KEY_GAMEPAD_LEFT_THUMB_UP, 8);

MAKE_INDEX(KEY_GAMEPAD_LEFT_THUMB_RIGHT, 9);

MAKE_INDEX(KEY_GAMEPAD_LEFT_THUMB_DOWN, 10);

MAKE_INDEX(KEY_GAMEPAD_RIGHT_THUMB, 11);

MAKE_INDEX(KEY_GAMEPAD_RIGHT_THUMB_LEFT, 12);

MAKE_INDEX(KEY_GAMEPAD_RIGHT_THUMB_UP, 13);

MAKE_INDEX(KEY_GAMEPAD_RIGHT_THUMB_RIGHT, 14);

MAKE_INDEX(KEY_GAMEPAD_RIGHT_THUMB_DOWN, 15);

MAKE_INDEX(KEY_GAMEPAD_LEFT_SHOULDER, 16);

MAKE_INDEX(KEY_GAMEPAD_RIGHT_SHOULDER, 17);

MAKE_INDEX(KEY_GAMEPAD_LEFT_TRIGGER, 18);

MAKE_INDEX(KEY_GAMEPAD_RIGHT_TRIGGER, 19);

MAKE_INDEX(KEY_GAMEPAD_A, 20);

MAKE_INDEX(KEY_GAMEPAD_B, 21);

MAKE_INDEX(KEY_GAMEPAD_X, 22);

MAKE_INDEX(KEY_GAMEPAD_Y, 23);

#undef MAKE_INDEX

class GamePadKeyRepeater {
	class ButtonState {
	public:
		core::time_millis downTime;
		core::time_millis updatedTime;
		bool down = false;
	};

	GamePad* gamepad = nullptr;
	ButtonState states[24];

	void update(core::Window* window, KeyCode kc, unsigned int index, bool down, core::time_millis time);
public:
	GamePadKeyRepeater();
	~GamePadKeyRepeater();

	void reset();
	void clearInput();
	void setGamePad(GamePad* gamepad);

	template<KeyCode Key>
	void update(core::Window* window, bool down, core::time_millis time) {
		this->update(window, Key, GamePadKeyRepeaterButtonIndexer<Key>::INDEX, down, time);
	}
};

}
// namespace userapp

#endif /* JNI_TEST_UTIL_GAMEPADKEYREPEATER_H_ */
