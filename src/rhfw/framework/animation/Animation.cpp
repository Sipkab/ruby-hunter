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
 * Animation.cpp
 *
 *  Created on: 2014.08.22.
 *      Author: sipka
 */

#include <framework/animation/Animation.h>
#include <framework/xml/XmlAttributes.h>

#include <framework/core/timing.h>

#include <gen/configuration.h>
#include <gen/xmldecl.h>
#include <gen/log.h>
namespace rhfw {

Animation::Animation(const xml::XmlAttributes& attrs)
		: state(AnimationState::CREATED), options(AnimationOptions::NO_FLAG), starttime(-1), duration(
				attrs.getOrDefault<unsigned int>(RXml::Attr::duration, 0)) {
}
Animation::Animation()
		: state(AnimationState::CREATED), options(AnimationOptions::NO_FLAG), starttime(-1), duration(0) {
}
Animation::Animation(core::time_millis duration)
		: state(AnimationState::CREATED), options(AnimationOptions::NO_FLAG), starttime(-1), duration(duration) {
}
Animation::Animation(core::time_millis start, core::time_millis duration)
		: state(AnimationState::CREATED), options(AnimationOptions::NO_FLAG), starttime(start), duration(duration) {
}
Animation::~Animation() {
}

void Animation::onTimeChanged(const core::time_millis& time, const core::time_millis& previous) {
	update_time_start:
	ASSERT(starttime != core::time_millis {-1}) << "trying to update animation without valid start time: -1";

	switch (state) {
		case AnimationState::CREATED: {
			if (time >= starttime) {
				onStart();
				state = AnimationState::STARTED;
			} else {
				break;
			}
			/*FALL-THROUGH*/
		}
			//no break (against warnings)
		case AnimationState::STARTED: {
			if (time >= starttime + duration) {
				onProgress(starttime + duration);

				//end of animation
				if (HAS_FLAG(options, AnimationOptions::NO_DELETE)) {
					if (HAS_FLAG(options, AnimationOptions::AUTO_RESTART)) {
						onFinish();
						state = AnimationState::CREATED;
						starttime = starttime + duration; //start where we ended

						//goto start if there is time left, play again
						goto update_time_start;
					} else {
						TimeListener::unsubscribe();
						LinkedNode < Animation > ::removeLinkFromList();
						//static_cast<*>(this)->removeLinkFromList();
						state = AnimationState::FINISHED;
						onFinish();
					}
				} else {
					TimeListener::unsubscribe();
					//remove link from list MUST be before onFinish()
					//onFinish can cause cascaded deletion via LifeCycleChain, so pointer "this" could be deleted twice
					//this can cause a lof of debugging
					LinkedNode < Animation > ::removeLinkFromList();
					onFinish();
					delete this;
				}
			} else {
				onProgress(time);
			}
			break;
		}
		case AnimationState::FINISHED: {

			break;
		}
		default:
			break;
	}
}

}
