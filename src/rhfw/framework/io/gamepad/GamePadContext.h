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
 * GamePadContext.h
 *
 *  Created on: 2017. aug. 15.
 *      Author: sipka
 */

#ifndef JNI_FRAMEWORK_IO_GAMEPAD_GAMEPADCONTEXT_H_
#define JNI_FRAMEWORK_IO_GAMEPAD_GAMEPADCONTEXT_H_

#include <gen/fwd/types.h>

namespace rhfw {
class GamePad;

class GamePadStateListener {
public:
	virtual ~GamePadStateListener() = default;

	virtual void onGamePadAttached(GamePad* gamepad) = 0;
	virtual void onGamePadDetached(GamePad* gamepad) = 0;
};

class GamePadContext {
public:
private:
public:
	virtual ~GamePadContext() = default;

	virtual void addGamePadStateListener(GamePadStateListener* listener) = 0;
	virtual void removeGamePadStateListener(GamePadStateListener* listener) = 0;

	virtual unsigned int getGamePadCount() = 0;
	virtual GamePad* getGamePad(unsigned int index) = 0;
};

}  // namespace rhfw

#endif /* JNI_FRAMEWORK_IO_GAMEPAD_GAMEPADCONTEXT_H_ */
