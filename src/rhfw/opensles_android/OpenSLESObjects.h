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
 * OpenSLESObjects.h
 *
 *  Created on: 2016. marc. 4.
 *      Author: sipka
 */

#ifndef OPENSLESOBJECTS_H_
#define OPENSLESOBJECTS_H_

#include <framework/utils/LinkedNode.h>

#include <SLES/OpenSLES.h>

#include <gen/log.h>
#include <gen/configuration.h>
#include <gen/fwd/types.h>

#define CHECK_OPENSLES_ERROR() ASSERT(result == SL_RESULT_SUCCESS) << "OpenSLES call failed, error: " << (unsigned long)result;
#define WARN_OPENSLES_ERROR() WARN(result != SL_RESULT_SUCCESS) << "OpenSLES call failed, error: " << (unsigned long)result;

namespace rhfw {
namespace audio {

template<typename InterfaceType>
class OpenSLESInterface {
private:
	InterfaceType itf;
public:
	OpenSLESInterface(InterfaceType itf = nullptr)
			: itf { itf } {
	}

	decltype(*itf) operator->() {
		ASSERT(itf != nullptr) << "Nullpointer exception";
		return *itf;
	}

	operator InterfaceType() {
		return itf;
	}

	bool operator!=(NULLPTR_TYPE) {
		return itf != nullptr;
	}
	bool operator==(NULLPTR_TYPE) {
		return itf == nullptr;
	}
};

class OpenSLESObject {
private:
	SLObjectItf object;
public:

	OpenSLESObject(SLObjectItf object = nullptr)
			: object { object } {
	}
	OpenSLESObject(const OpenSLESObject&) = delete;
	OpenSLESObject& operator=(const OpenSLESObject&) = delete;
	OpenSLESObject(OpenSLESObject&& o)
			: object { o.object } {
		o.object = nullptr;
	}
	OpenSLESObject& operator=(OpenSLESObject&& o) {
		ASSERT(this->object != o.object) << "self move assignment";
		this->object = o.object;
		o.object = nullptr;
		return *this;
	}
	~OpenSLESObject() {
		ASSERT(object == nullptr) << "OpenSLES object was not destroyed";
	}
	SLObjectItf* operator&() {
		return &object;
	}

	operator SLObjectItf() {
		return object;
	}

	template<typename SLType>
	operator SLType();

	template<typename SlItfType>
	operator OpenSLESInterface<SlItfType>() {
		return (SlItfType) *this;
	}

	decltype(*object) operator->() {
		return *object;
	}

	SLresult realize();

	void destroy() {
		if (object != nullptr) {
			(*object)->Destroy(object);
			object = nullptr;
		}
	}
};

}  // namespace audio
}  // namespace rhfw

#endif /* OPENSLESOBJECTS_H_ */
