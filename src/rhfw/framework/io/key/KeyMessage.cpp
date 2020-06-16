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
 * KeyMessage.cpp
 *
 *  Created on: 2015 aug. 28
 *      Author: sipka
 */

#include <framework/io/key/KeyMessage.h>
#include <framework/io/key/KeyEvent.h>

#include <gen/log.h>

namespace rhfw {

bool KeyMessage::postMessage(core::Window* window, InputDevice inputdevice, KeyAction action, const ExtraData& data) {
	ASSERT(action != KeyAction::DOWN || action != KeyAction::UP || data.keycode != KeyCode::KEY_UNKNOWN);
	//LOGV()<<"KeyMessage: device: " << inputdevice << ", action: "<< action << " key: " << data.keycode;

	KeyMessage msg;

	msg.set(window, inputdevice, action, data);
	return KeyEvent::instance.dispatchMessage(&msg);
}

}
