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
 * ScrollGestureDetector.cpp
 *
 *  Created on: 2015 �pr. 6
 *      Author: sipka
 */

#include <framework/io/touch/gesture/scroll/ScrollGestureDetector.h>
#include <framework/io/touch/TouchEvent.h>

#include <gen/log.h>

#define _USE_MATH_DEFINES
#include <math.h>

namespace rhfw {

static void limit(float& dim, float min, float max) {
//	ASSERT(min - max < 0.1f) << min << " - " << max;
	if (dim < min) {
		dim = min;
	}
	if (dim > max) {
		dim = max;
	}
}

class ScrollGestureDetector::WheelAnimator: public ScrollGestureDetector::ScrollAnimation {
private:
public:
	float a = 1.0f;
	float b = 0.0f;
	Vector2F& property;
	Vector2F target;

	Vector2F pstart;
	Vector2F pdiff;

	WheelAnimator(Vector2F& property, const Vector2F& target, core::time_millis starttime, core::time_millis duration)
			: ScrollAnimation(false), property(property), target(target) {
		this->starttime = starttime;
		this->duration = duration;
	}
	virtual void onStart() override {
		pstart = property;
		pdiff = target - pstart;
	}
	virtual void onFinish() override {
		property = target;
	}

	void onProgress(const core::time_millis& progress) override {
		const float percent = 1 - (float) ((progress - starttime) / duration);
		float eq = 1 - (a * percent * percent + b * percent);
		core::time_millis pprogress = starttime + core::time_millis { (long long) ((eq * (float) (long long) duration)) };
		const float ppercent = (float) ((pprogress - starttime) / duration);
		property = pstart + pdiff * ppercent;
	}
};
class ScrollGestureDetector::FlingAnimation: public ScrollAnimation {
public:
	ScrollGestureDetector& detector;
	Vector2F startPos;
	Vector2F velocity;

	FlingAnimation(ScrollGestureDetector& detector, const Vector2F& start, const Vector2F& velocity)
			: ScrollAnimation(true), detector(detector), startPos(start), velocity(velocity) {
	}

	virtual void onProgress(const core::time_millis& progress) override {
		const float percent = (float) ((progress - Animation::starttime) / Animation::duration);
		//detector.scrollpos = start + velocity * (percent * 10);
		detector.scrollpos = startPos + velocity * (1 - pow(M_E, -1 * 5 * percent));

		limit(detector.scrollpos.x(), detector.size.width() - detector.workingSize.width(), 0);
		limit(detector.scrollpos.y(), detector.size.height() - detector.workingSize.height(), 0);
	}
};

ScrollGestureDetector::ScrollGestureDetector() {
}
ScrollGestureDetector::~ScrollGestureDetector() {
}

//void limit(Vector2F& dim, const Vector2F& min, const Vector2F& max) {
//	limit(dim.x(), min.x(), max.x());
//	limit(dim.y(), min.y(), max.y());
//}
void ScrollGestureDetector::setSize(const Vector2F& size, const Vector2F& workingsize) {
	//		ASSERT(size.width() - workingsize.width() <= 0.5) << size.width() << " - " << workingsize.width();
	//		ASSERT(size.height() - workingsize.height() <= 0.5) << size.height() << " - " << workingsize.height();
	this->size = size;
	this->workingSize = workingsize;
	limit(scrollpos.x(), size.width() - workingSize.width(), 0);
	limit(scrollpos.y(), size.height() - workingSize.height(), 0);
}
void ScrollGestureDetector::onTouch(bool applyscroll) {
	Vector2F middle = TouchEvent::instance.getCenter();

	switch (TouchEvent::instance.getAction()) {
		case TouchAction::DOWN:
			cancelFling();
			touchPositionCount = 0;
			if (TouchEvent::instance.getPointerCount() == 1) {
				addTouchPosition(middle, TouchEvent::instance.getTime());
			}
			break;
		case TouchAction::MOVE_UPDATE: {
			cancelFling();
			if (applyscroll) {
				scrollpos += (middle - downMiddle) * dragMultiplier;
				limit(scrollpos.x(), size.width() - workingSize.width(), 0);
				limit(scrollpos.y(), size.height() - workingSize.height(), 0);
			}
			if (TouchEvent::instance.getPointerCount() == 1) {
				addTouchPosition(middle, TouchEvent::instance.getTime());
			} else {
				touchPositionCount = 0;
			}
			break;
		}
		case TouchAction::UP: {
			if (TouchEvent::instance.getPointerCount() == 0) {
				if (applyscroll && touchPositionCount >= 3 && !isFlingHoldTimeElapsed(TouchEvent::instance.getTime())) {
					cancelFling();
					auto vec = calculateFlingVelocity(TouchEvent::instance.getTime()) * 32 * dragMultiplier;
					FlingAnimation* fling = new FlingAnimation(*this, scrollpos, vec);
					fling->setDuration(core::time_millis { 1000 });
					fling->setStartTime(TouchEvent::instance.getTime());
					animHolder.link(fling);
					fling->start();
				}
			} else {
				touchPositionCount = 0;
			}
			break;
		}
		case TouchAction::CANCEL: {
			cancelFling();
			touchPositionCount = 0;
			break;
		}
		case TouchAction::WHEEL: {
			if (applyscroll) {
				float percent = TouchEvent::instance.getExtra().getWheelPercent();
				applyWheel(percent);
			}
			touchPositionCount = 0;
			break;
		}
		case TouchAction::SCROLL: {
			if (applyscroll) {
				scrollpos += TouchEvent::instance.getExtra().getScroll() * dragMultiplier;
				limit(scrollpos.x(), size.width() - workingSize.width(), 0);
				limit(scrollpos.y(), size.height() - workingSize.height(), 0);
			}
			break;
		}
		default: {
			break;
		}
	}
	downMiddle = middle;
}
void ScrollGestureDetector::applyWheel(float percent) {
	Vector2F move = wheelMultiplier * percent;
	animateWheel(move);
}
void ScrollGestureDetector::animateTo(Vector2F target) {
	limit(target.x(), size.width() - workingSize.width(), 0);
	limit(target.y(), size.height() - workingSize.height(), 0);

	ScrollAnimation* anim = static_cast<ScrollAnimation*>(animHolder.get());

	auto start = core::MonotonicTime::getCurrent();
	float wheellen = wheelMultiplier.length();
	float durmult = wheellen > 0.1f ? (scrollpos - target).length() / wheellen : 1.0f;
	if (durmult > 1.0f) {
		durmult = 1.0f;
	}
	if (durmult < 0.1f) {
		durmult = 0.1f;
	}
	core::time_millis duration { (long long) (175 * durmult) };
	auto* newanim = new WheelAnimator(scrollpos, target, start, duration);
	if (anim != nullptr) {
		if (!anim->fling) {
			WheelAnimator* wa = static_cast<WheelAnimator*>(anim);
			const float percent = (float) ((start - wa->getStartTime()) / wa->getDuration());

			newanim->b = (2 * wa->a * percent + wa->b);
			newanim->a = -newanim->b + 1;

		}
		animHolder.kill();
	}
	animHolder.link(newanim);
	newanim->start();
}
void ScrollGestureDetector::animateWheel(const Vector2F& move) {
	Vector2F target;
	ScrollAnimation* anim = static_cast<ScrollAnimation*>(animHolder.get());
	if (anim != nullptr) {
		if (anim->fling) {
			cancelFling();
			target = scrollpos + move;
		} else {
			WheelAnimator* wa = static_cast<WheelAnimator*>(anim);
			target = wa->target + move;
		}
	} else {
		target = scrollpos + move;
	}
	animateTo(target);
}
void ScrollGestureDetector::scaleAtPoint(float factor, const Vector2F& point) {
	//modify scrollpos
	//Vector2F topoint = point - scrollpos;
	//Size pointfract = topoint / workingSize;
	//LOGD("Scaleatpoint factor: %f", factor);
	Vector2F pointfract = (point - scrollpos) /= workingSize;
	this->workingSize = size * factor;

	scrollpos = point - workingSize * pointfract;
}

float ScrollGestureDetector::getTargetScrollPosition(float scroll, float size, float workingsize, float left, float right) {
	float width = right - left;
	if (right + scroll > size) {
		return -(right - size);
	}
	if (left + scroll < 0) {
		return -left;
	}
	return scroll;
}

float ScrollGestureDetector::getTargetScrollPosition(float scroll, float size, float workingsize, float left, float right, float padleft,
		float padright) {
	float width = right - left;
	if (right + scroll > size - padright) {
		return -right + size - padright;
	}
	if (left + scroll < padleft) {
		return -left + padleft;
	}
	return scroll;
}

Vector2F ScrollGestureDetector::getTargetScrollPosition(const Rectangle& makevisible) {
	return Vector2F { getTargetScrollPosition(scrollpos.x(), size.width(), workingSize.width(), makevisible.left, makevisible.right),
			getTargetScrollPosition(scrollpos.y(), size.height(), workingSize.height(), makevisible.top, makevisible.bottom) };
}

void ScrollGestureDetector::addTouchPosition(const Vector2F& pos, core::time_millis time) {
	ASSERT(touchPositionsIndex < MAX_TOUCH_POSITIONS);

	if (isFlingHoldTimeElapsed(time)) {
		touchPositionCount = 0;
	}

	auto&& tp = touchPositions[(touchPositionsIndex + touchPositionCount) % MAX_TOUCH_POSITIONS];
	tp.position = pos;
	tp.time = time;

	if (touchPositionCount < MAX_TOUCH_POSITIONS) {
		++touchPositionCount;
	} else {
		++touchPositionsIndex;
		if (touchPositionsIndex >= MAX_TOUCH_POSITIONS) {
			touchPositionsIndex = 0;
		}
	}
}

Vector2F ScrollGestureDetector::calculateFlingVelocity(core::time_millis time) {
	ASSERT(touchPositionCount >= 2);
	Vector2F direction { 0, 0 };
	float totaltime = ((long long) (time - touchPositions[touchPositionsIndex].time)) / 1000.0f;
	if (totaltime < 0.005f) {
		totaltime = 0.005f;
	}
	Vector2F diraccum { 0, 0 };
	for (unsigned int i = 0; i < touchPositionCount - 1; ++i) {
		auto&& current = touchPositions[(touchPositionsIndex + i) % MAX_TOUCH_POSITIONS];
		auto&& next = touchPositions[(touchPositionsIndex + i + 1) % MAX_TOUCH_POSITIONS];
		Vector2F dir = next.position - current.position;
		diraccum += dir;
		if (next.time != current.time) {
			float timediff = (((long long) (next.time - current.time)) / 1000.0f);
			direction += diraccum * (timediff / totaltime);
			diraccum = {0,0};
		}
	}
	return direction;
}

bool ScrollGestureDetector::isFlingHoldTimeElapsed(core::time_millis time) {
	if (touchPositionCount > 0) {
		auto diff = time - touchPositions[(touchPositionsIndex + touchPositionCount - 1) % MAX_TOUCH_POSITIONS].time;
		if (diff > core::time_millis { 100 }) {
			return true;
		}
	}
	return false;
}

Vector2F ScrollGestureDetector::getTargetScrollPosition(const Rectangle& makevisible, const Rectangle& paddings) {
	return Vector2F { getTargetScrollPosition(scrollpos.x(), size.width(), workingSize.width(), makevisible.left, makevisible.right,
			paddings.left, paddings.right), getTargetScrollPosition(scrollpos.y(), size.height(), workingSize.height(), makevisible.top,
			makevisible.bottom, paddings.top, paddings.bottom) };
}

} // namespace rhfw

