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
 * XInputGamePad.cpp
 *
 *  Created on: 2017. aug. 15.
 *      Author: sipka
 */

#include <xinput/XInputGamePad.h>
#include <xinput/XInputGamePadContext.h>
#include <xinput/XInputContextBase.h>

namespace rhfw {

XInputGamePad::XInputGamePad(XInputContextBase* context, uint32 gamepadindex)
		: GamePad(), context(context), gamepadId(gamepadindex) {
}
XInputGamePad::~XInputGamePad() {
}

bool XInputGamePad::getState(GamePadState* stateout) {
	XINPUT_STATE state;
	DWORD stateres = context->func_XInputGetState(gamepadId, &state);
	switch (stateres) {
		case ERROR_SUCCESS: {
			GamePadState& result = *stateout;

			//GamePadButtons enum must be the same as XInput button definitions
			result.buttons = (GamePadButtons) state.Gamepad.wButtons;
			result.triggerLeft = state.Gamepad.bLeftTrigger;
			result.triggerRight = state.Gamepad.bRightTrigger;
			result.thumbLeftX = state.Gamepad.sThumbLX;
			result.thumbLeftY = state.Gamepad.sThumbLY;
			result.thumbRightX = state.Gamepad.sThumbRX;
			result.thumbRightY = state.Gamepad.sThumbRY;
			return true;
		}
		case ERROR_DEVICE_NOT_CONNECTED: {
			LOGI()<<"XInput device not connected " << gamepadId;
			//TODO handle disconnection?
			context->reportGamePadGetStateNotConnected(this);
			return false;
		}
		default: {
			LOGI() << "Unknown XInputGetState error: " << stateres;
			return false;
		}
	}
}

}
 // namespace rhfw

