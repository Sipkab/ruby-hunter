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
 * TouchEvent.h
 *
 *  Created on: 2014.08.30.
 *      Author: sipka
 */

#ifndef TOUCHEVENT_H_
#define TOUCHEVENT_H_

#include <framework/geometry/Vector.h>
#include <framework/core/Window.h>
#include <framework/core/timing.h>

#include <gen/types.h>
#include <gen/configuration.h>

namespace rhfw {

class TouchPointer {
	friend class TouchEvent;
private:
	bool down;
	Vector2F pos { 0.0f, 0.0f };
	Vector2F prevpos { 0.0f, 0.0f };

	unsigned int id;

	InputDevice inputDevice = InputDevice::UNKNOWN;
public:
	TouchPointer()
			: down(false), id(-1) {
	}
	~TouchPointer() {
	}

	bool isDown() const {
		return down;
	}
	const Vector2F& getPosition() const {
		return pos;
	}
	const Vector2F& getPreviousPosition() const {
		return prevpos;
	}
	unsigned int getId() const {
		return id;
	}

	InputDevice getInputDevice() const {
		return inputDevice;
	}
};

class TouchEvent {
	friend class TouchMessage;
public:
	static const int MAX_POINTER_COUNT = 16;

	class ExtraData {
	public:
		float scrollHorizontal;
		float scrollVertical;

		int wheel;
		int wheelMax;

		float getWheelPercent() const {
			return (float) wheel / wheelMax;
		}

		Vector2F getScroll() const {
			return Vector2F { scrollHorizontal, scrollVertical };
		}
	};
private:
	TouchAction action = TouchAction::CANCEL;
	unsigned int validPointerCount;
	TouchPointer pointers[MAX_POINTER_COUNT];
	TouchPointer* affectedPointer;

	core::time_millis time;

	ExtraData extra;

	TouchEvent()
			: validPointerCount(0), affectedPointer(nullptr) {
		for (int i = 0; i < MAX_POINTER_COUNT; ++i) {
			pointers[i].id = i;
		}
	}
	~TouchEvent() {
	}

	TouchEvent(const TouchEvent& other);
	void operator=(const TouchEvent& other);

	void pointerDown(core::Window* window, core::time_millis time, int id, float x, float y, const ExtraData& extra);
	void pointerUp(core::Window* window, core::time_millis time, int id, float x, float y, const ExtraData& extra);
	void pointerMoveUpdate(core::Window* window, core::time_millis time, int id, float x, float y);
	void pointerMoveUpdatesDone(core::Window* window, const ExtraData& extra);
	void pointerMovedSingle(core::Window* window, core::time_millis time, int id, float x, float y, const ExtraData& extra);
	void pointerHoverSingle(core::Window* window, core::time_millis time, int id, float x, float y, const ExtraData& extra);
	void cancelEvent(core::Window* window, core::time_millis time, const ExtraData& extra);
	void wheelEvent(core::Window* window, core::time_millis time, int id, float x, float y, const ExtraData& extra);
	void scrollEvent(core::Window* window, core::time_millis time, int id, float x, float y, const ExtraData& extra);
public:
	static TouchEvent instance;

	TouchAction getAction() const {
		return action;
	}
	TouchPointer* getPointer(int id);
	unsigned int getPointerCount() const {
		return validPointerCount;
	}
	/**
	 * Only valid on actions: ACTION_DOWN, ACTION_UP
	 */
	TouchPointer* getAffectedPointer() {
		return affectedPointer;
	}
	bool isJustDown() const {
		return validPointerCount == 1 && action == TouchAction::DOWN;
	}
	bool isJustUp() const {
		return validPointerCount == 0 && action == TouchAction::UP;
	}
	core::time_millis getTime() const {
		return time;
	}

	Vector2F getCenter() const;

	/**
	 * fromId inclusive
	 */
	TouchPointer* findNextPointer(unsigned int fromId = 0);

	const ExtraData& getExtra() const {
		return extra;
	}

};
}

#endif /* TOUCHEVENT_H_ */
