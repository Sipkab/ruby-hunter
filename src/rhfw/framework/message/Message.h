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
 * Message.h
 *
 *  Created on: 2015 febr. 23
 *      Author: sipka
 */

#ifndef MESSAGE_H_
#define MESSAGE_H_

#include <framework/utils/LinkedNode.h>
#include <framework/core/timing.h>

#include <gen/configuration.h>
#include <gen/types.h>

namespace rhfw {

class Message: public LinkedNode<Message>, public core::TimeListener {
private:
	MessageOptions options;
protected:
	virtual bool dispatchMessageImpl() = 0;
public:
	Message(MessageOptions options = MessageOptions::NO_FLAG)
			: options(options) {
	}
	virtual ~Message() {
	}

	void onTimeChanged(const core::time_millis& time, const core::time_millis& previous) override final {
		MessageOptions options = this->options;
		if (dispatchMessageImpl()) {
			if (!HAS_FLAG(options, MessageOptions::NO_DELETE)) {
				delete this;
			} else {
				LinkedNode < Message > ::removeLinkFromList();
			}
		}
	}

	void post() {
		core::GlobalMonotonicTimeListener::subscribeListener(*this);
	}

	Message* get() override {
		return this;
	}

	MessageOptions getOptions() const {
		return options;
	}

	void setOptions(MessageOptions options) {
		this->options = options;
	}
};
} // namespace rhfw

#endif /* MESSAGE_H_ */
