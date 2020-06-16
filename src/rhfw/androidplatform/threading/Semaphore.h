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
 * AndroidSemaphore.h
 *
 *  Created on: 2015 aug. 10
 *      Author: sipka
 */

#ifndef ANDROIDSEMAPHORE_H_
#define ANDROIDSEMAPHORE_H_

#include <semaphore.h>
#include <errno.h>

#include <gen/configuration.h>
#include <gen/log.h>

namespace rhfw {

class AndroidSemaphore {
public:
	class auto_init {
	};
private:
	sem_t semaphore;
	bool valid;
public:
	AndroidSemaphore()
			: valid { false } {
	}
	AndroidSemaphore(auto_init)
			: AndroidSemaphore() {
		init();
	}
	AndroidSemaphore(const AndroidSemaphore& o) = delete;
	AndroidSemaphore& operator=(const AndroidSemaphore& o) = delete;
	AndroidSemaphore(AndroidSemaphore&& o)
			: semaphore(o.semaphore), valid { o.valid } {
		o.valid = false;
	}
	AndroidSemaphore& operator=(AndroidSemaphore&& o) {
		ASSERT(this != &o) << "Self move assignment";

		this->semaphore = o.semaphore;
		this->valid = o.valid;

		o.valid = false;
		return *this;
	}
	~AndroidSemaphore() {
		if (valid) {
			int res = sem_destroy(&semaphore);
			ASSERT(res == 0) << "syscall failed, result: " << res << ", errno: " << strerror(errno);
		}
	}

	void init() {
		ASSERT(!isValid()) << "Trying to initialize sempahore more than once";

		int res = sem_init(&semaphore, 0, 0);
		ASSERT(res == 0) << "syscall failed, result: " << res << ", errno: " << strerror(errno);
		valid = true;
	}
	void destroy() {
		ASSERT(isValid()) << "semaphore is not valid";

		int res = sem_destroy(&semaphore);
		ASSERT(res == 0) << "syscall failed, result: " << res << ", errno: " << strerror(errno);
		valid = false;
	}
	void post() {
		ASSERT(isValid()) << "semaphore is not valid";

		int res = sem_post(&semaphore);
		ASSERT(res == 0) << "syscall failed, result: " << res << ", errno: " << strerror(errno);
	}
	void wait() {
		ASSERT(isValid()) << "semaphore is not valid";

		int res = sem_wait(&semaphore);
		ASSERT(res == 0) << "syscall failed, result: " << res << ", errno: " << strerror(errno);
	}
	bool isValid() const {
		return valid;
	}

};

} // namespace rhfw

#endif /* ANDROIDSEMAPHORE_H_ */
