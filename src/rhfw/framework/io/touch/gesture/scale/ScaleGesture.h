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
 * GestureDetector.h
 *
 *  Created on: 2014.08.31.
 *      Author: sipka
 */

#ifndef GESTUREDETECTOR_H_
#define GESTUREDETECTOR_H_

#include <framework/geometry/Vector.h>
#include <gen/configuration.h>
namespace rhfw {

class TouchEvent;
class Boundary;
class TouchPointer;

typedef void (*ScaleCallback)(float scaleFactor, void* param);
class ScaleGestureDetector {
private:
	TouchPointer* pointerA = nullptr;
	TouchPointer* pointerB = nullptr;

	float startScaleDistance = 0.0f;

	float prevscale = 1.0f;
	float scale = 1.0f;

	Vector2F center;

	ScaleCallback callback;
	void* callbackParam;

public:
	ScaleGestureDetector(ScaleCallback callback = nullptr, void* callbackParam = nullptr)
			: center { 0.0f, 0.0f }, callback(callback), callbackParam(callbackParam) {
	}
	~ScaleGestureDetector() {
	}

	void onTouch();

	void setCallback(ScaleCallback callback, void* param = nullptr) {
		this->callback = callback;
		this->callbackParam = param;
	}

	const Vector2F& getFocus() const {
		return center;
	}

	bool isValid() const {
		return true;
	}
	void cancel() {
	}

	float getScale() const {
		return scale;
	}

	float getPreviousScale() const {
		return prevscale;
	}
};
}

#endif /* GESTUREDETECTOR_H_ */
