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
 * GestureDetector.cpp
 *
 *  Created on: 2014.08.31.
 *      Author: sipka
 */

#include <framework/io/touch/gesture/scale/ScaleGesture.h>
#include <framework/io/touch/TouchEvent.h>
#include <framework/geometry/Boundary.h>
#include <math.h>
#include <gen/log.h>
#include <gen/types.h>
namespace rhfw {

#define calculateStartScaleDistance() ASSERT(pointerA != nullptr && pointerB != nullptr) << "one of the pointers are nullptr";\
	startScaleDistance = pointerA->getPosition().distance(pointerB->getPosition())

void ScaleGestureDetector::onTouch() {
	TouchEvent& event = TouchEvent::instance;
	switch (event.getAction()) {
		case TouchAction::DOWN: {
			const unsigned int pointerCount = event.getPointerCount();
			switch (pointerCount) {
				case 1:
					pointerA = event.getAffectedPointer();
					break;
				default:
					if (pointerB == nullptr) {
						ASSERT(pointerA != nullptr) << "pointerA is nullptr";
						pointerB = event.getAffectedPointer();
						calculateStartScaleDistance();
					} else if (pointerA == nullptr) {
						ASSERT(pointerB != nullptr) << "pointerB is nullptr";
						pointerA = event.getAffectedPointer();
						calculateStartScaleDistance();
					}
					break;
			}
			break;
		}
		case TouchAction::MOVE_UPDATE: {
			if (pointerB == nullptr || pointerA == nullptr) {
				return;
			}
			const float distance = pointerA->getPosition().distance(pointerB->getPosition());

			//TODO ha inkonzisztens a touch event income, a startScaleDistance lehet 0.0f, �s factor NaN lesz.

			const float factor = distance / startScaleDistance;
			startScaleDistance = distance;

			prevscale = scale;
			scale *= factor;

			if (callback != nullptr) {
				center = pointerA->getPosition();
				center += pointerB->getPosition();
				center /= 2.0f;
				callback(scale, callbackParam);
			}

			break;
		}
		case TouchAction::UP: {
			TouchPointer* affected = event.getAffectedPointer();
			if (affected == pointerA) {
				//pointerA was released, rename B to A
				pointerA = pointerB;
				goto UP_SEARCH_NEW_PTR_LABEL;
			} else if (affected == pointerB) {
				goto UP_SEARCH_NEW_PTR_LABEL;
			}
			break;

			UP_SEARCH_NEW_PTR_LABEL: {
				//search new value for pointerB
				const unsigned int count = event.getPointerCount();
				if (count > 0) {
					if (pointerA == nullptr) {
						pointerA = event.findNextPointer();
						if (count > 1) {
							pointerB = event.findNextPointer(pointerA->getId() + 1);
							calculateStartScaleDistance();
						} else {
							pointerB = nullptr;
						}
					} else {
						if (count > 1) {
							pointerB = event.findNextPointer(pointerA->getId() + 1);
							calculateStartScaleDistance();
						} else {
							pointerB = nullptr;
						}
					}
				}
			}
			break;
		}
		case TouchAction::CANCEL: {
			pointerA = nullptr;
			pointerB = nullptr;
			break;
		}
		default:
			break;
	}
}
}
