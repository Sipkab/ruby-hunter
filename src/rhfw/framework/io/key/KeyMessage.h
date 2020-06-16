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
 * KeyMessage.h
 *
 *  Created on: 2015 aug. 28
 *      Author: sipka
 */

#ifndef KEYMESSAGE_H_
#define KEYMESSAGE_H_

#include <framework/core/Window.h>
#include <framework/io/key/UnicodeCodePoint.h>

#include <gen/configuration.h>
#include <gen/types.h>

namespace rhfw {
class GamePad;
class KeyMessage {
	friend class KeyEvent;
public:
	class ExtraData {
	public:
		//TODO make this really different
		/**
		 * Valid on action UP, DOWN
		 */
		KeyCode keycode = KeyCode::KEY_UNKNOWN;
		KeyModifiers modifiers = KeyModifiers::NO_FLAG;
		//valid only on DOWN
		bool repeat = false;
		/**
		 * Valid on action UNICODE_SEQUENCE
		 */
		UnicodeCodePoint* sequenceUnicode = nullptr;
		unsigned int sequenceUnicodeCount = 0;
		/**
		 * Valid on action UNICODE_REPEAT
		 */
		UnicodeCodePoint repeatUnicode = 0;
		unsigned int repeatUnicodeCount = 0;

		GamePad* gamePad = nullptr;
	};
private:
	core::Window* window = nullptr;
	InputDevice inputdevice = InputDevice::UNKNOWN;
	KeyAction action;

	ExtraData extra;

	KeyMessage() {
	}
	~KeyMessage() = default;

	void set(core::Window* window, InputDevice inputdevice, KeyAction action, const ExtraData& data) {
		this->window = window;
		this->inputdevice = inputdevice;
		this->action = action;
		this->extra = data;
	}
protected:
public:
	static bool postMessage(core::Window* window, InputDevice inputdevice, KeyAction action, const ExtraData& data);

};

}

#endif /* KEYMESSAGE_H_ */
