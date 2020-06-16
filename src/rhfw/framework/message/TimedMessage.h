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
 * TimedMessage.h
 *
 *  Created on: 2015 aug. 10
 *      Author: sipka
 */

#ifndef TIMEDMESSAGE_H_
#define TIMEDMESSAGE_H_

#include <framework/message/Message.h>

#include <framework/core/timing.h>

namespace rhfw {

class TimedMessage: public Message {
private:
protected:
	virtual bool dispatchMessageImpl() override {
		if (core::MonotonicTime::getCurrent() >= deliveryTime) {
			return true;
		}
		return false;
	}
	core::time_millis deliveryTime;
public:
	TimedMessage(core::time_millis deliveryTime = core::time_millis { 0 }, MessageOptions options = MessageOptions::NO_FLAG)
			: Message(options), deliveryTime(deliveryTime) {
	}
	virtual ~TimedMessage() {
	}

	void setDeliveryTime(core::time_millis time) {
		this->deliveryTime = time;
	}

	TimedMessage* get() override {
		return this;
	}
};
}

#endif /* TIMEDMESSAGE_H_ */
