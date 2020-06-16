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
 * Mutex.h
 *
 *  Created on: 2015 aug. 10
 *      Author: sipka
 */

#ifndef LINUXMUTEX_H_
#define LINUXMUTEX_H_

#include <pthread.h>
#include <errno.h>
#include <string.h>

#include <gen/configuration.h>
#include <gen/log.h>

namespace rhfw {
class LinuxMutex {
public:
	class auto_init {
	};
private:
	pthread_mutex_t mutex;
	bool valid;
public:
	LinuxMutex()
			: valid { false } {
	}
	LinuxMutex(auto_init)
			: LinuxMutex() {
		init();
	}
	LinuxMutex(const LinuxMutex& o) = delete;
	LinuxMutex& operator=(const LinuxMutex& o) = delete;
	LinuxMutex(LinuxMutex&& o)
			: mutex(o.mutex), valid { o.valid } {
		o.valid = false;
	}
	LinuxMutex& operator=(LinuxMutex&& o) {
		ASSERT(this != &o) << "Self move assignment";

		this->mutex = o.mutex;
		this->valid = o.valid;

		o.valid = false;
		return *this;
	}
	~LinuxMutex() {
		if (valid) {
			int res = pthread_mutex_destroy(&mutex);
			ASSERT(res == 0) << "syscall failed, result: " << res << ", errno: " << strerror(errno);
		}
	}

	void init() {
		ASSERT(!isValid()) << "Trying to initialize mutex more than once";

		int res;
		pthread_mutexattr_t attr;

		res = pthread_mutexattr_init(&attr);
		ASSERT(res == 0) << "syscall failed, result: " << res << ", errno: " << strerror(errno);
		res = pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
		ASSERT(res == 0) << "syscall failed, result: " << res << ", errno: " << strerror(errno);

		res = pthread_mutex_init(&mutex, &attr);
		ASSERT(res == 0) << "syscall failed, result: " << res << ", errno: " << strerror(errno);
		res = pthread_mutexattr_destroy(&attr);
		valid = true;
	}
	void destroy() {
		ASSERT(isValid()) << "mutex is not valid";

		int res = pthread_mutex_destroy(&mutex);
		ASSERT(res == 0) << "syscall failed, result: " << res << ", errno: " << strerror(errno);
		valid = false;
	}

	bool isValid() const {
		return valid;
	}

	void lock() {
		ASSERT(isValid()) << "mutex is not valid";

		int res = pthread_mutex_lock(&mutex);
		ASSERT(res == 0) << "syscall failed, result: " << res << ", errno: " << strerror(errno);
	}
	void unlock() {
		ASSERT(isValid()) << "mutex is not valid";

		int res = pthread_mutex_unlock(&mutex);
		ASSERT(res == 0) << "syscall failed, result: " << res << ", errno: " << strerror(errno);
	}
};

}

#endif /* LINUXMUTEX_H_ */
