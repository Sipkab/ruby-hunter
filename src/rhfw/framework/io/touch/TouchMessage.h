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
 * TouchMessage.h
 *
 *  Created on: 2015 m�rc. 30
 *      Author: sipka
 */

#ifndef TOUCHMESSAGE_H_
#define TOUCHMESSAGE_H_

#include <framework/core/Window.h>
#include <framework/core/timing.h>
#include <framework/io/touch/TouchEvent.h>

#include <gen/configuration.h>
#include <gen/types.h>

namespace rhfw {

class TouchMessage {
public:

private:
	TouchMessage() {
	}
	~TouchMessage() = default;
protected:
public:

	static void postMessage(core::Window* targetWindow, InputDevice inputdevice, int id, TouchAction action, float x, float y,
			core::time_millis time, const TouchEvent::ExtraData& extra = { });

};
}

#endif /* TOUCHMESSAGE_H_ */
