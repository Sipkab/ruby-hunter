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
 * Semaphore.h
 *
 *  Created on: 2015 aug. 10
 *      Author: sipka
 */

#ifndef APPLESEMAPHORE_H_
#define APPLESEMAPHORE_H_

#include <dispatch/dispatch.h>
#include <gen/configuration.h>
#include <gen/log.h>

namespace rhfw {

class AppleSemaphore {
public:
	class auto_init {
	};
private:
	dispatch_semaphore_t semaphore;
	bool valid;
public:
	AppleSemaphore()
			: valid { false } {
	}
	AppleSemaphore(auto_init)
			: AppleSemaphore() {
		init();
	}
	~AppleSemaphore() {
		if (valid) {
#if !OS_OBJECT_USE_OBJC
			dispatch_release(semaphore);
#endif
		}
	}

	void init() {
		ASSERT(!isValid()) << "Trying to initialize sempahore more than once";
		semaphore = dispatch_semaphore_create(0);

		ASSERT(semaphore != nullptr) << "Failed to create semaphore";

		valid = true;
	}
	void destroy() {
		ASSERT(isValid()) << "semaphore is not valid";
#if !OS_OBJECT_USE_OBJC
		dispatch_release(semaphore);
#endif
		valid = false;
	}
	void post() {
		ASSERT(isValid()) << "semaphore is not valid";
		dispatch_semaphore_signal(semaphore);
	}
	void wait() {
		ASSERT(isValid()) << "semaphore is not valid";

		dispatch_semaphore_wait(semaphore, DISPATCH_TIME_FOREVER);
	}
	bool isValid() const {
		return valid;
	}

};

} // namespace rhfw

#endif /* APPLESEMAPHORE_H_ */
