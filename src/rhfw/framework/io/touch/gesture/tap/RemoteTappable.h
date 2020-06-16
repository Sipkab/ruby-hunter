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
 * RemoteTappable.h
 *
 *  Created on: 2015 m�rc. 7
 *      Author: sipka
 */

#ifndef REMOTETAPPABLE_H_
#define REMOTETAPPABLE_H_

#include <framework/io/touch/gesture/tap/Tappable.h>
#include <framework/io/touch/gesture/tap/TapGesture.h>
#include <gen/configuration.h>
namespace rhfw {

class RemoteTappable;

typedef void (*TapCallback)(const Tappable::TapEvent& tapdata, void* target, RemoteTappable* source);

class RemoteTappable: public Tappable {
private:
	TapCallback callback;
	void* taptarget;
protected:
	TapGestureDetector tapdetector;
public:
	RemoteTappable()
			: callback(nullptr), taptarget(nullptr) {
	}
	virtual ~RemoteTappable() {
	}
	virtual void onTap(const TapEvent& pos) override {
		if (callback != nullptr) {
			callback(pos, taptarget, this);
		}
	}

	void setTapCallback(TapCallback callback, void* target) {
		this->callback = callback;
		this->taptarget = target;
	}
};
}

#endif /* REMOTETAPPABLE_H_ */
