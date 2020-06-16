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
 * GamePadKeyRepeater.cpp
 *
 *  Created on: 2017. aug. 22.
 *      Author: sipka
 */

#include <framework/io/key/KeyMessage.h>
#include <gen/log.h>
#include <sapphire/util/GamePadKeyRepeater.h>

namespace userapp {
using namespace rhfw;

#define KEYBOARD_DELAY_MS 440
#define KEYBOARD_REPEAT_MS 35

GamePadKeyRepeater::GamePadKeyRepeater() {
}

GamePadKeyRepeater::~GamePadKeyRepeater() {
}

void GamePadKeyRepeater::reset() {
	this->gamepad = nullptr;
	clearInput();
}
void GamePadKeyRepeater::clearInput() {
	for (auto&& s : states) {
		s.down = false;
	}
}

void GamePadKeyRepeater::update(core::Window* window, KeyCode kc, unsigned int index, bool down, core::time_millis time) {
	ASSERT(gamepad != nullptr);

	ButtonState& state = states[index];
	if (down) {
		if (state.down) {
			long long downdiff = (long long) (time - state.downTime);
			if (downdiff >= KEYBOARD_DELAY_MS) {
				long long diff = (long long) (time - state.updatedTime);
				while (diff > KEYBOARD_REPEAT_MS) {
					diff -= KEYBOARD_REPEAT_MS;
					KeyMessage::ExtraData extra;
					extra.gamePad = gamepad;
					extra.keycode = kc;
					extra.repeat = true;
					LOGI()<< extra.keycode << " repeat";
					KeyMessage::postMessage(window, InputDevice::GAMEPAD, KeyAction::DOWN, extra);
					state.updatedTime += core::time_millis { 40 };
				}
			}
		} else {
			state.down = true;
			state.downTime = time;
			state.updatedTime = time + core::time_millis { KEYBOARD_DELAY_MS };
		}
	} else {
		state.down = false;
	}
}

void GamePadKeyRepeater::setGamePad(GamePad* gamepad) {
	this->gamepad = gamepad;
	clearInput();
}

} // namespace userapp
