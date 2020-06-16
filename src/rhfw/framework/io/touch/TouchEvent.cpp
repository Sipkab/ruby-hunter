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
 * TouchEvent.cpp
 *
 *  Created on: 2014.08.30.
 *      Author: sipka
 */

#include <framework/io/touch/TouchEvent.h>

#include <gen/log.h>
namespace rhfw {

TouchEvent TouchEvent::instance;

void TouchEvent::pointerDown(core::Window* window, core::time_millis time, int id, float x, float y, const ExtraData& extra) {
	ASSERT(id >= 0 && id < MAX_POINTER_COUNT) << "invalid touch id: " << id;

	this->time = time;

	affectedPointer = pointers + id;

	TouchPointer& ptr = pointers[id];
	if (ptr.down) {
		LOGW()<< "pointer was not up for going down: "<< ptr.id;
		//inconsistent touch input

		cancelEvent(window, time, ExtraData {});
		return;
	}
	this->extra = extra;
	ptr.prevpos = ptr.pos = Vector2F { x, y };
	ptr.down = true;

	++validPointerCount;

	action = TouchAction::DOWN;

	window->dispatchTouchEvent();
}
void TouchEvent::pointerMoveUpdate(core::Window* window, core::time_millis time, int id, float x, float y) {
	ASSERT(id >= 0 && id < MAX_POINTER_COUNT) << "invalid touch id: " << id;

	this->time = time;

	affectedPointer = pointers + id;

	TouchPointer& ptr = pointers[id];
	if (!ptr.down) {
		LOGW()<<"pointer was not down for move: " << ptr.id;
		//inconsistent touch input

		cancelEvent(window, time, ExtraData {});
		return;
	}
	ptr.prevpos = ptr.pos;
	ptr.pos = Vector2F { x, y };

	action = TouchAction::MOVE_UPDATE;
}
void TouchEvent::pointerMoveUpdatesDone(core::Window* window, const ExtraData& extra) {
	this->extra = extra;
	window->dispatchTouchEvent();
}
void TouchEvent::pointerMovedSingle(core::Window* window, core::time_millis time, int id, float x, float y, const ExtraData& extra) {
	ASSERT(id >= 0 && id < MAX_POINTER_COUNT) << "invalid touch id: " << id;

	this->time = time;

	affectedPointer = pointers + id;

	TouchPointer& ptr = pointers[id];
	if (!ptr.down) {
		LOGW()<< "pointer was not down for move: " << ptr.id;
		//inconsistent touch input

		cancelEvent(window, time, ExtraData {});
		return;
	}
	this->extra = extra;
	ptr.prevpos = ptr.pos;
	ptr.pos = Vector2F { x, y };

	action = TouchAction::MOVE_UPDATE;

	window->dispatchTouchEvent();
}
void TouchEvent::pointerUp(core::Window* window, core::time_millis time, int id, float x, float y, const ExtraData& extra) {
	ASSERT(id >= 0 && id < MAX_POINTER_COUNT) << "invalid touch id: " << id;

	this->time = time;

	TouchPointer& ptr = pointers[id];
	if (!ptr.down) {
		LOGW()<<"pointer was not down for going up: " << ptr.id;
		//inconsistent touch input
		cancelEvent(window, time, ExtraData {});
		return;
	}
	this->extra = extra;
	ptr.prevpos = ptr.pos;
	ptr.pos = Vector2F { x, y };
	ptr.down = false;

	--validPointerCount;

	affectedPointer = pointers + id;

	action = TouchAction::UP;

	window->dispatchTouchEvent();
}
void TouchEvent::cancelEvent(core::Window* window, core::time_millis time, const ExtraData& extra) {
	LOGTRACE()<< "Cancel touch event";
	this->time = time;
	this->extra = extra;

	action = TouchAction::CANCEL;
	for (int i = 0; i < MAX_POINTER_COUNT; ++i) {
		pointers[i].down = false;
	}
	validPointerCount = 0;
	affectedPointer = nullptr;

	window->dispatchTouchEvent();
}
void TouchEvent::wheelEvent(core::Window* window, core::time_millis time, int id, float x, float y, const ExtraData& extra) {
	this->time = time;

	affectedPointer = pointers + id;

	this->extra = extra;

	action = TouchAction::WHEEL;

	TouchPointer& ptr = pointers[id];
	//don't set prevpos, as it will matter on move update
//	ptr.prevpos = ptr.pos;
	ptr.pos = Vector2F { x, y };

	window->dispatchTouchEvent();
}
void TouchEvent::scrollEvent(core::Window* window, core::time_millis time, int id, float x, float y, const ExtraData& extra) {
	this->time = time;

	affectedPointer = pointers + id;

	this->extra = extra;

	action = TouchAction::SCROLL;

	TouchPointer& ptr = pointers[id];
	//don't set prevpos, as it will matter on move update
//	ptr.prevpos = ptr.pos;
	ptr.pos = Vector2F { x, y };

	window->dispatchTouchEvent();
}

void TouchEvent::pointerHoverSingle(core::Window* window, core::time_millis time, int id, float x, float y, const ExtraData& extra) {
	ASSERT(id >= 0 && id < MAX_POINTER_COUNT) << "invalid touch id: " << id;

	this->time = time;

	affectedPointer = pointers + id;

	TouchPointer& ptr = pointers[id];
	this->extra = extra;
	ptr.prevpos = ptr.pos;
	ptr.pos = Vector2F { x, y };

	action = TouchAction::HOVER_MOVE;

	window->dispatchTouchEvent();
}

TouchPointer* TouchEvent::findNextPointer(unsigned int fromId) {
	if (validPointerCount > 0) {
		for (unsigned int i = 0; i < MAX_POINTER_COUNT; ++i) {
			TouchPointer* ptr = pointers + (fromId + i) % MAX_POINTER_COUNT;
			if (ptr->isDown()) {
				return ptr;
			}
		}
	}
	return nullptr;
}

TouchPointer* TouchEvent::getPointer(int id) {
	ASSERT(id >= 0 && id < MAX_POINTER_COUNT) << "invalid touch id: " << id;
	return pointers + id;
}

Vector2F TouchEvent::getCenter() const {
	Vector2F result { 0.0f, 0.0f };
	if (validPointerCount > 0) {
		for (int i = 0; i < MAX_POINTER_COUNT; ++i) {
			const TouchPointer* ptr = pointers + i;
			if (ptr->isDown()) {
				result += ptr->pos;
			}
		}
		result /= (float) validPointerCount;
	}
	return result;
}

}

