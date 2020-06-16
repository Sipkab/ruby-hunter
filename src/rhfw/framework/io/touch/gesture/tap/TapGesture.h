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
 * TapGesture.h
 *
 *  Created on: 2015 febr. 23
 *      Author: sipka
 */

#ifndef TAPGESTURE_H_
#define TAPGESTURE_H_

#include <framework/message/Message.h>
#include <framework/utils/LifeCycleChain.h>

#include <gen/types.h>
#include <gen/configuration.h>
namespace rhfw {

class TouchEvent;
class TouchPointer;
class Layer;
class Tappable;

class TapGestureDetector {
private:
	friend class TapMessage;

	LifeCycleChain<Message> holder;
	TapData enabledTapTypes = TapData::SINGLE_TAP;

	bool valid = false;
	TouchPointer* tapPointer = nullptr;

	void cancelDuringMessageDelivery() {
		holder.unlink();
		valid = false;
	}

public:
	TapGestureDetector() {
	}
	~TapGestureDetector() {
	}

	void onTouch(Tappable* source);

	void setEnabledTapTypes(TapData enabled) {
		this->enabledTapTypes = enabled;
	}

	bool isValid() const {
		return valid;
	}
	void cancel() {
		holder.kill();
		valid = false;
	}
};
}

#endif /* TAPGESTURE_H_ */
