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
 * KeyEvent.h
 *
 *  Created on: 2015 aug. 28
 *      Author: sipka
 */

#ifndef KEYEVENT_H_
#define KEYEVENT_H_

#include <framework/io/key/KeyMessage.h>

#include <gen/configuration.h>
#include <gen/types.h>
#include <gen/log.h>

namespace rhfw {

class KeyEvent {
	friend class KeyMessage;
private:
	KeyMessage* current = nullptr;

	KeyEvent() {
	}
	~KeyEvent() {
	}

	bool dispatchMessage(KeyMessage* msg);
public:
	static KeyEvent instance;

	KeyAction getAction() const {
		return current->action;
	}

	KeyCode getKeycode() const {
		return current->extra.keycode;
	}

	KeyModifiers getModifiers() const {
		return current->extra.modifiers;
	}

	InputDevice getInputDevice() const {
		return current->inputdevice;
	}

	GamePad* getGamePad() const {
		return current->extra.gamePad;
	}

	bool isRepeat() const {
		ASSERT(getAction() == KeyAction::DOWN);
		return current->extra.repeat;
	}

	const UnicodeCodePoint* getUnicodeSequence() const {
		ASSERT(getAction() == KeyAction::UNICODE_SEQUENCE);
		return current->extra.sequenceUnicode;
	}
	unsigned int getUnicodeSequenceLength() const {
		ASSERT(getAction() == KeyAction::UNICODE_SEQUENCE);
		return current->extra.sequenceUnicodeCount;
	}

	UnicodeCodePoint getUnicodeRepeat() const {
		ASSERT(getAction() == KeyAction::UNICODE_REPEAT);
		return current->extra.repeatUnicode;
	}
	unsigned int getUnicodeRepeatCount() const {
		ASSERT(getAction() == KeyAction::UNICODE_REPEAT);
		return current->extra.repeatUnicodeCount;
	}
};

}

#endif /* KEYEVENT_H_ */
