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
 * PropertyAnimator.h
 *
 *  Created on: 2015 febr. 15
 *      Author: sipka
 */

#ifndef PROPERTYANIMATOR_H_
#define PROPERTYANIMATOR_H_

#include <framework/animation/Animation.h>
#include <gen/configuration.h>
namespace rhfw {

template<typename T>
class PropertyAnimator: public Animation {
protected:
	T& property;
	T pstart;
	T ptarget;
	T pdiff;

	virtual void onFinish() override {
		property = ptarget;
	}
public:
	PropertyAnimator(T& property, const T& target)
			: Animation(), property(property), ptarget(target) {
	}
	PropertyAnimator(T& property, const T& target, core::time_millis start)
			: Animation(start), property(property), ptarget(target) {
	}
	PropertyAnimator(T& property, const T& target, core::time_millis start, core::time_millis duration)
			: Animation(start, duration), property(property), ptarget(target) {
	}
	~PropertyAnimator() {
	}
	virtual void onStart() override {
		pstart = property;
		pdiff = ptarget - pstart;
	}
	virtual void onProgress(const core::time_millis& progress) override {
		const float percent = (float) ((progress - starttime) / duration);
		property = pstart + pdiff * percent;
	}
};
}

#endif /* PROPERTYANIMATOR_H_ */
