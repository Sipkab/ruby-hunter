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
 * LinuxRandomContext.cpp
 *
 *  Created on: 2016. okt. 7.
 *      Author: sipka
 */

#include <linuxplatform/random/LinuxRandomContext.h>
#include <framework/random/PseudoRandomer.h>
#include <framework/core/timing.h>

#include <time.h>

namespace rhfw {

class LinuxRandomer final: public Randomer {
	LinuxRandomContext* context;
public:
	LinuxRandomer(LinuxRandomContext* context)
			: context(context) {
	}

	virtual int read(void* buffer, unsigned int count) override {
		return context->input->read(buffer, count);
	}
};

LinuxRandomContext::LinuxRandomContext() {
}
LinuxRandomContext::~LinuxRandomContext() {
	ASSERT(input == nullptr) << "RandomContext was not freed";
}

bool LinuxRandomContext::load() {
	input = randomFile.createInput();
	if (input->open()) {
	} else {
		delete input;
		input = nullptr;
	}
	return true;
}

void LinuxRandomContext::free() {
	if (input != nullptr) {
		input->close();
		delete input;
		input = nullptr;
	}
}

Randomer* LinuxRandomContext::createRandomer() {
	if (input != nullptr) {
		return new LinuxRandomer(this);
	}
	struct timespec currentTime { 0 };
	int res = clock_gettime(CLOCK_MONOTONIC, &currentTime);
	return new PseudoRandomer(
			(long long) static_cast<long long>(currentTime.tv_sec) * 1000 + static_cast<long long>(currentTime.tv_nsec) / 1000000);
}

} // namespace rhfw

