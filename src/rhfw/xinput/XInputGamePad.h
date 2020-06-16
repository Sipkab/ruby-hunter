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
 * XInputGamePad.h
 *
 *  Created on: 2017. aug. 15.
 *      Author: sipka
 */

#ifndef JNI_XINPUT_GAMEPAD_XINPUTGAMEPAD_H_
#define JNI_XINPUT_GAMEPAD_XINPUTGAMEPAD_H_

#include <framework/io/gamepad/GamePad.h>

#include <win32platform/minwindows.h>
#include <Xinput.h>

namespace rhfw {
class XInputGamePadContext;
class XInputContextBase;
class XInputGamePad: public GamePad {
private:
	XInputContextBase* context;
	const uint32 gamepadId;
public:
	XInputGamePad(XInputContextBase* context, uint32 gamepadindex);
	~XInputGamePad();

	virtual int32 getThumbMin() override {
		return -32768;
	}
	virtual int32 getThumbMax() override {
		return 32767;
	}

	virtual uint32 getTriggerMax() override {
		return 255;
	}

	virtual int32 getLeftThumbDeadzone() override {
		return XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE;
	}
	virtual int32 getRightThumbDeadzone() {
		return XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE;
	}
	virtual int32 getTriggerDeadzone() override {
		return XINPUT_GAMEPAD_TRIGGER_THRESHOLD;
	}

	virtual bool getState(GamePadState* stateout) override;

	uint32 getGamePadId() const {
		return gamepadId;
	}
};

} // namespace rhfw

#endif /* JNI_XINPUT_GAMEPAD_XINPUTGAMEPAD_H_ */
