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
 * TouchMessage.cpp
 *
 *  Created on: 2015 m�rc. 30
 *      Author: sipka
 */

#include <framework/io/touch/TouchMessage.h>
#include <framework/io/touch/TouchEvent.h>

#include <gen/types.h>
#include <gen/log.h>

namespace rhfw {

void TouchMessage::postMessage(core::Window* targetWindow, InputDevice inputdevice, int id, TouchAction action, float x, float y,
		core::time_millis time, const TouchEvent::ExtraData& extra) {
	//TODO handle inputdevice

	switch (action) {
		case TouchAction::DOWN: {
			TouchEvent::instance.pointerDown(targetWindow, time, id, x, y, extra);
			break;
		}
		case TouchAction::UP: {
			TouchEvent::instance.pointerUp(targetWindow, time, id, x, y, extra);
			break;
		}
		case TouchAction::MOVE_UPDATE: {
			TouchEvent::instance.pointerMoveUpdate(targetWindow, time, id, x, y);
			//no dispatch, batched calls
			break;
		}
		case TouchAction::MOVE_UPDATE_DONE: {
			TouchEvent::instance.pointerMoveUpdatesDone(targetWindow, extra);
			break;
		}
		case TouchAction::CANCEL: {
			TouchEvent::instance.cancelEvent(targetWindow, time, extra);
			break;
		}
		case TouchAction::MOVED_SINGLE: {
			TouchEvent::instance.pointerMovedSingle(targetWindow, time, id, x, y, extra);
			break;
		}
		case TouchAction::WHEEL: {
			TouchEvent::instance.wheelEvent(targetWindow, time, id, x, y, extra);
			break;
		}
		case TouchAction::HOVER_MOVE: {
			TouchEvent::instance.pointerHoverSingle(targetWindow, time, id, x, y, extra);
			break;
		}
		case TouchAction::SCROLL: {
			TouchEvent::instance.scrollEvent(targetWindow, time, id, x, y, extra);
			break;
		}
		default: {
			THROW()<<"Invalid touchevent action: " << action;
			break;
		}
	}
}

}
 // namespace rhfw
