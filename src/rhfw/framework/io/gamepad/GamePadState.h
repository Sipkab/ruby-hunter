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
 * GamePadState.h
 *
 *  Created on: 2017. aug. 15.
 *      Author: sipka
 */

#ifndef JNI_FRAMEWORK_IO_GAMEPAD_GAMEPADSTATE_H_
#define JNI_FRAMEWORK_IO_GAMEPAD_GAMEPADSTATE_H_

#include <gen/fwd/types.h>

namespace rhfw {

class GamePadState {
private:
public:
	GamePadButtons buttons;

	uint32 triggerLeft;
	uint32 triggerRight;
	int32 thumbLeftX;
	int32 thumbLeftY;
	int32 thumbRightX;
	int32 thumbRightY;

	void reset() {
		buttons = (GamePadButtons) 0;
		triggerLeft = 0;
		triggerRight = 0;
		thumbLeftX = 0;
		thumbLeftY = 0;
		thumbRightX = 0;
		thumbRightY = 0;
	}
};

}  // namespace rhfw

#endif /* JNI_FRAMEWORK_IO_GAMEPAD_GAMEPADSTATE_H_ */
