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
 * Animation.h
 *
 *  Created on: 2014.08.22.
 *      Author: sipka
 */

#ifndef ANIMATION_H_
#define ANIMATION_H_

#include <framework/utils/LinkedNode.h>
#include <framework/core/timing.h>

#include <gen/types.h>
#include <gen/configuration.h>

namespace rhfw {

namespace xml {
class XmlAttributes;
}  // namespace xml

class Animation;

class Animation: public LinkedNode<Animation>, public core::TimeListener {
private:
	AnimationState state;
protected:
	AnimationOptions options;

	core::time_millis starttime;
	core::time_millis duration;

	//progress clamped to [start, end] region
	virtual void onProgress(const core::time_millis& progress) = 0;
	virtual void onStart() {
	}
	virtual void onFinish() {
	}
public:
	Animation(const xml::XmlAttributes& attrs);
	Animation();
	Animation(core::time_millis duration);
	Animation(core::time_millis start, core::time_millis duration);
	virtual ~Animation();

	core::time_millis getDuration() const {
		return duration;
	}
	void setDuration(core::time_millis duration) {
		this->duration = duration;
	}

	core::time_millis getStartTime() const {
		return starttime;
	}
	void setStartTime(core::time_millis start) {
		this->starttime = start;
	}

	//previously started animation will remain unfinished
	void start() {
		this->state = AnimationState::CREATED;
		core::GlobalMonotonicTimeListener::subscribeListener(*this);
	}

	void onTimeChanged(const core::time_millis& time, const core::time_millis& previous) override final;

	Animation* get() override {
		return this;
	}

	AnimationOptions getOptions() const {
		return options;
	}
	void setOptions(AnimationOptions options) {
		this->options = options;
	}
};
}

#endif /* ANIMATION_H_ */
