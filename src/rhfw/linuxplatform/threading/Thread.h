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
 * Thread.h
 *
 *  Created on: 2015 aug. 10
 *      Author: sipka
 */

#ifndef LINUXTHREAD_H_
#define LINUXTHREAD_H_

#include <framework/threading/Thread.h>
#include <framework/utils/LinkedNode.h>

#include <gen/configuration.h>
#include <gen/log.h>

#include <pthread.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <time.h>

#include <stdlib.h>
#include <errno.h>
#include <string.h>

namespace rhfw {

class LinuxThread: public ThreadBase {
public:
	typedef int (*ThreadProcFunc)(void* args);
	typedef void (*VoidThreadProcFunc)(void* args);

	typedef pid_t Id;

	static Id getCurrentId() {
		return syscall(SYS_gettid);
	}
	/**
	 * Returns the remaining time, because the sleep was interrupted
	 */
	static unsigned int sleep(unsigned int millis) {
		timespec req;
		timespec rem;
		req.tv_sec = millis / 1000;
		req.tv_nsec = (millis % 1000) * 1000000;
		int res = nanosleep(&req, &rem);
		if (res == 0) {
			return 0;
		}
		ASSERT(errno == EINTR) << errno;
		if (errno == EINTR) {
			return rem.tv_sec * 1000 + rem.tv_nsec / 1000000;
		}
		return 0;
	}

private:
	friend class platform_bridge;

	template<typename T>
	static void* threadProcFunctor(void* arg) {
		T functor = *reinterpret_cast<T*>(arg);
		delete reinterpret_cast<T*>(arg);

		void* res = (void*) (size_t) (functor());

		return res;
	}

	class ThreadProcArgsData {
	public:
		ThreadProcFunc function;
		void* arg;
	};
	static void* threadProcArgs(void* arg) {
		ThreadProcArgsData data = *reinterpret_cast<ThreadProcArgsData*>(arg);
		delete reinterpret_cast<ThreadProcArgsData*>(arg);

		void* result = (void*) (size_t) data.function(data.arg);

		return result;
	}

	class VoidThreadProcArgsData {
	public:
		VoidThreadProcFunc function;
		void* arg;
	};
	static void* threadProcArgsIgnoreReturn(void* arg) {
		VoidThreadProcArgsData data = *reinterpret_cast<VoidThreadProcArgsData*>(arg);
		delete reinterpret_cast<VoidThreadProcArgsData*>(arg);

		data.function(data.arg);

		return nullptr;
	}

	pthread_t thread = 0;

public:
	LinuxThread() {
	}
	LinuxThread(ThreadProcFunc threadproc, void* arg) {
		start(threadproc, arg);
	}
	LinuxThread(VoidThreadProcFunc threadproc, void* arg) {
		start(threadproc, arg);
	}
	template<typename T>
	LinuxThread(const T& functor) {
		start(functor);
	}
	//TODO destroy thread handle?
	void start(ThreadProcFunc threadproc, void* arg) {
		int res;
		pthread_attr_t attr;

		res = pthread_attr_init(&attr);
		ASSERT(res == 0) << "syscall failed, result: " << res << ", errno: " << strerror(errno);
		res = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
		ASSERT(res == 0) << "syscall failed, result: " << res << ", errno: " << strerror(errno);
		res = pthread_create(&thread, &attr, threadProcArgs, new ThreadProcArgsData { threadproc, arg });
		ASSERT(res == 0) << "syscall failed, result: " << res << ", errno: " << strerror(errno);
		res = pthread_attr_destroy(&attr);
		ASSERT(res == 0) << "syscall failed, result: " << res << ", errno: " << strerror(errno);
	}
	void start(VoidThreadProcFunc threadproc, void* arg) {
		int res;
		pthread_attr_t attr;

		res = pthread_attr_init(&attr);
		ASSERT(res == 0) << "syscall failed, result: " << res << ", errno: " << strerror(errno);
		res = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
		ASSERT(res == 0) << "syscall failed, result: " << res << ", errno: " << strerror(errno);
		res = pthread_create(&thread, &attr, threadProcArgsIgnoreReturn, new VoidThreadProcArgsData { threadproc, arg });
		ASSERT(res == 0) << "syscall failed, result: " << res << ", errno: " << strerror(errno);
		res = pthread_attr_destroy(&attr);
		ASSERT(res == 0) << "syscall failed, result: " << res << ", errno: " << strerror(errno);
	}
	template<typename T>
	void start(const T& functor) {
		int res;
		pthread_attr_t attr;

		res = pthread_attr_init(&attr);
		ASSERT(res == 0) << "syscall failed, result: " << res << ", errno: " << strerror(errno);
		res = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
		ASSERT(res == 0) << "syscall failed, result: " << res << ", errno: " << strerror(errno);
		res = pthread_create(&thread, &attr, threadProcFunctor<T>, new T(functor));
		ASSERT(res == 0) << "syscall failed, result: " << res << ", errno: " << strerror(errno);
		res = pthread_attr_destroy(&attr);
		ASSERT(res == 0) << "syscall failed, result: " << res << ", errno: " << strerror(errno);
	}

	void* join() {
		void* retval;
		int res = pthread_join(thread, &retval);
		ASSERT(res == 0) << "syscall failed, result: " << res << ", errno: " << strerror(errno);
		return retval;
	}
};

}
// namespace rhfw

#endif /* LINUXTHREAD_H_ */
