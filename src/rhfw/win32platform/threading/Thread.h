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

#ifndef WIN32THREAD_H_
#define WIN32THREAD_H_

#include <win32platform/minwindows.h>
#include <framework/threading/Thread.h>

#include <gen/configuration.h>
#include <gen/log.h>

namespace rhfw {

class Win32Thread: public ThreadBase {
public:
	typedef int (*ThreadProcFunc)(void* args);
	typedef void (*VoidThreadProcFunc)(void* args);

	typedef decltype(GetCurrentThreadId()) Id;

	static Id getCurrentId() {
		return GetCurrentThreadId();
	}
	/**
	 * Returns the remaining time, because the sleep was interrupted
	 */
	static unsigned int sleep(unsigned int millis) {
		Sleep((DWORD) millis);
		return 0;
	}
	void yield() {
		Sleep(0);
	}

private:

	template<typename T>
	static DWORD WINAPI threadProcFunctor(void* arg) {
		T functor = *reinterpret_cast<T*>(arg);
		delete reinterpret_cast<T*>(arg);

		return (DWORD) functor();
	}

	class ThreadProcArgsData {
	public:
		ThreadProcFunc function;
		void* arg;
	};
	static DWORD WINAPI threadProcArgs(void* arg) {
		ThreadProcArgsData data = *reinterpret_cast<ThreadProcArgsData*>(arg);
		delete reinterpret_cast<ThreadProcArgsData*>(arg);
		int result = data.function(data.arg);

		return result;
	}

	class VoidThreadProcArgsData {
	public:
		VoidThreadProcFunc function;
		void* arg;
	};
	static DWORD WINAPI threadProcArgsIgnoreReturn(void* arg) {
		VoidThreadProcArgsData data = *reinterpret_cast<VoidThreadProcArgsData*>(arg);
		delete reinterpret_cast<VoidThreadProcArgsData*>(arg);
		data.function(data.arg);

		return 0;
	}

public:
	void start(ThreadProcFunc threadproc, void* arg) {
		HANDLE handle = CreateThread(nullptr, 0, threadProcArgs, new ThreadProcArgsData { threadproc, arg }, 0, nullptr);
		ASSERT(handle != NULL) << "Failed to start thread, error: " << GetLastError();
		CloseHandle(handle);
	}
	void start(VoidThreadProcFunc threadproc, void* arg) {
		HANDLE handle = CreateThread(nullptr, 0, threadProcArgsIgnoreReturn, new VoidThreadProcArgsData { threadproc, arg }, 0, nullptr);
		ASSERT(handle != NULL) << "Failed to start thread, error: " << GetLastError();
		CloseHandle(handle);
	}
	template<typename T>
	void start(const T& functor) {
		HANDLE handle = CreateThread(nullptr, 0, threadProcFunctor<T>, new T { functor }, 0, nullptr);
		ASSERT(handle != NULL) << "Failed to start thread, error: " << GetLastError();
		CloseHandle(handle);
	}
	//TODO when adding join method here, dont forget to make the user able to create joinable pthreads on appropriate platforms
};

}

#endif /* WIN32THREAD_H_ */
