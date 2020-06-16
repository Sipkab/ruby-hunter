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
 * MacOsxRandomContext.cpp
 *
 *  Created on: 2016. okt. 7.
 *      Author: sipka
 */

#include <macosxplatform/random/MacOsxRandomContext.h>
#include <framework/random/PseudoRandomer.h>
#include <framework/core/timing.h>

namespace rhfw {

class MacOsxRandomer final: public Randomer {
	MacOsxRandomContext* context;
public:
	MacOsxRandomer(MacOsxRandomContext* context)
			: context(context) {
	}

	virtual int read(void* buffer, unsigned int count) override {
		return context->input->read(buffer, count);
	}
};

MacOsxRandomContext::MacOsxRandomContext() {
}
MacOsxRandomContext::~MacOsxRandomContext() {
	ASSERT(input == nullptr) << "RandomContext was not freed";
}

bool MacOsxRandomContext::load() {
	input = randomFile.createInput();
	if (input->open()) {
	} else {
		delete input;
		input = nullptr;
	}
	return true;
}

void MacOsxRandomContext::free() {
	if (input != nullptr) {
		input->close();
		delete input;
		input = nullptr;
	}
}

Randomer* MacOsxRandomContext::createRandomer() {
	if (input != nullptr) {
		return new MacOsxRandomer(this);
	}
	return new PseudoRandomer((long long) core::MonotonicTime::getCurrent());
}

} // namespace rhfw

