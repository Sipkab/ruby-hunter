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
 * TapGesture.cpp
 *
 *  Created on: 2015 febr. 23
 *      Author: sipka
 */

#include <gen/log.h>
#include <framework/io/touch/gesture/tap/TapGesture.h>
#include <framework/io/touch/TouchEvent.h>
#include <framework/io/touch/gesture/tap/Tappable.h>

#include <framework/message/TimedMessage.h>
#include <gen/types.h>
namespace rhfw {

//TODO fix, ha gyorsan lerak up utan, akkor cancelezodik a message

class TapMessage: public TimedMessage {
public:
	TapGestureDetector& detector;
	Tappable* source;
	Tappable::TapEvent event;

	TapMessage(TapGestureDetector& detector, core::time_millis deliveryTime, Tappable* source, const Vector2F& pos, TapData tapdata)
			: TimedMessage(deliveryTime), detector(detector), source(source), event(pos, tapdata) {
	}

	bool dispatchMessageImpl() override {
		bool result = TimedMessage::dispatchMessageImpl();
		if (result) {
			detector.cancelDuringMessageDelivery();
			source->onTap(event);
		}
		return result;
	}
};

void TapGestureDetector::onTouch(Tappable* source) {
	TouchEvent& event = TouchEvent::instance;
	ASSERT(source != nullptr) << "source is null for ontouch";

	switch (event.getAction()) {
		case TouchAction::DOWN: {
			ASSERT(event.getPointerCount() > 0) << "not enough pointers?";
			if (event.getPointerCount() == 1) {
				tapPointer = event.getAffectedPointer();
				valid = source != nullptr && source->isInside(tapPointer->getPosition());
				if (HAS_FLAG(enabledTapTypes, TapData::LONG_TAP)) {
					holder.kill();

					holder.link(
							new TapMessage(*this, event.getTime() + core::time_millis { 300 }, source, tapPointer->getPosition(),
									TapData::LONG_TAP));
					holder.get()->post();
				}
			} else {
				holder.kill();
				valid = false;
			}
			break;
		}
		case TouchAction::MOVE_UPDATE: {
			if (!valid)
				break;
			if (source == nullptr || !source->isInside(tapPointer->getPosition())) {
				holder.kill();
				valid = false;
			}
			break;
		}
		case TouchAction::UP: {
			if (event.getAffectedPointer() != tapPointer) {
				holder.kill();
				valid = false;
			}
			if (!valid)
				break;
			ASSERT(event.getAffectedPointer() == tapPointer) << "not the tappointer going up with valid gesture?";
			ASSERT(!tapPointer->isDown()) << "tappointer is still down";
			holder.kill();
			valid = false;

			if (HAS_FLAG(enabledTapTypes, TapData::SINGLE_TAP)) {
				holder.link(new TapMessage(*this, event.getTime(), source, tapPointer->getPosition(), TapData::SINGLE_TAP));
				holder.get()->post();
			}

			break;
		}
		case TouchAction::CANCEL: {
			holder.kill();
			valid = false;
			break;
		}
		default:
			break;
	}
}
}
