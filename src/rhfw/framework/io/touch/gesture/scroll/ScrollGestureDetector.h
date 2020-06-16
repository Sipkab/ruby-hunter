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
 * ScrollGestureDetector.h
 *
 *  Created on: 2015 �pr. 6
 *      Author: sipka
 */

#ifndef SCROLLGESTUREDETECTOR_H_
#define SCROLLGESTUREDETECTOR_H_

#include <framework/utils/LifeCycleChain.h>
#include <framework/animation/Animation.h>
#include <framework/geometry/Rectangle.h>

#include <framework/geometry/Vector.h>
#include <gen/configuration.h>
#include <gen/log.h>
namespace rhfw {

class ScrollGestureDetector {
private:
	class ScrollAnimation: public Animation {
	public:
		bool fling;

		ScrollAnimation(bool fling)
				: fling(fling) {
		}
	};
	class FlingAnimation;
	class WheelAnimator;
	class FlingTouchPosition {
	public:
		Vector2F position;
		core::time_millis time;
	};
	/**
	 *
	 */
	static const unsigned int MAX_TOUCH_POSITIONS = 128;

	FlingTouchPosition touchPositions[MAX_TOUCH_POSITIONS];
	unsigned int touchPositionCount = 0;
	unsigned int touchPositionsIndex = 0;

	LifeCycleChain<Animation> animHolder;

	Vector2F size { 0.0f, 0.0f };
	Vector2F scrollpos { 0.0f, 0.0f };

	Vector2F workingSize { 0.0f, 0.0f };

	Vector2F velocity { 0.0f, 0.0f };

	Vector2F downMiddle { 0.0f, 0.0f };

	Vector2F wheelMultiplier { 0.0f, 0.0f };
	Vector2F dragMultiplier { 1.0f, 1.0f };

	static float getTargetScrollPosition(float scroll, float size, float workingsize, float left, float right);
	static float getTargetScrollPosition(float scroll, float size, float workingsize, float left, float right, float padleft,
			float padright);

	void animateWheel(const Vector2F& move);

	void addTouchPosition(const Vector2F& pos, core::time_millis time);

	bool isFlingHoldTimeElapsed(core::time_millis time);

	Vector2F calculateFlingVelocity(core::time_millis time);
public:
	ScrollGestureDetector();
	~ScrollGestureDetector();

	Rectangle getScrollRange() const {
		return Rectangle { 0, 0, size.width() - workingSize.width(), size.height() - workingSize.height() };
	}

	void onTouch(bool applyscroll = true);

	void setSize(const Vector2F& size, float scale) {
		setSize(size, size * scale);
	}
	void setSize(const Vector2F& size, const Vector2F& workingsize);
	void scaleAtPoint(float factor, const Vector2F& point);
	void modifySizeInPlace(const Vector2F& size);

	void applyWheel(float percent);
	void animateTo(Vector2F target);

	const Vector2F& getPosition() const {
		return scrollpos;
	}
	Vector2F& getPosition() {
		return scrollpos;
	}
	void cancelFling() {
		animHolder.kill();
	}
	bool isAnimating() {
		return animHolder.get() != nullptr;
	}

	const Vector2F& getWorkingSize() const {
		return workingSize;
	}
	const Vector2F& getSize() const {
		return size;
	}
	Vector2F getTargetScrollPosition(const Rectangle& makevisible);
	Vector2F getTargetScrollPosition(const Rectangle& makevisible, const Rectangle& paddings);

	void setWheelMultiplier(const Vector2F& mult) {
		this->wheelMultiplier = mult;
	}
	void setDragMultiplier(const Vector2F& mult) {
		this->dragMultiplier = mult;
	}
};
}

#endif /* SCROLLGESTUREDETECTOR_H_ */
