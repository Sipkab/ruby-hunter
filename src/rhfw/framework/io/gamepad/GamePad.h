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
 * GamePad.h
 *
 *  Created on: 2017. aug. 15.
 *      Author: sipka
 */

#ifndef JNI_FRAMEWORK_IO_GAMEPAD_GAMEPAD_H_
#define JNI_FRAMEWORK_IO_GAMEPAD_GAMEPAD_H_

#include <gen/fwd/types.h>
#include <framework/io/gamepad/GamePadState.h>

namespace rhfw {

class GamePad {
private:
protected:
public:
	GamePad() {
	}
	virtual ~GamePad() = default;

	virtual int32 getThumbMin() = 0;
	virtual int32 getThumbMax() = 0;

	virtual uint32 getTriggerMax() = 0;

	virtual bool getState(GamePadState* stateout) = 0;

	virtual int32 getLeftThumbDeadzone() = 0;
	virtual int32 getRightThumbDeadzone() = 0;
	virtual int32 getTriggerDeadzone() = 0;

};

}  // namespace rhfw

#endif /* JNI_FRAMEWORK_IO_GAMEPAD_GAMEPAD_H_ */
