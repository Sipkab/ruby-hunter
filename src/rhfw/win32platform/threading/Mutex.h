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

#ifndef WIN32MUTEX_H_
#define WIN32MUTEX_H_

#include <win32platform/minwindows.h>

#include <gen/configuration.h>
#include <gen/log.h>

namespace rhfw {
class Win32Mutex {
public:
	class auto_init {
	};
private:
	HANDLE handle = INVALID_HANDLE_VALUE;
public:
	~Win32Mutex() {
		if (isValid()) {
			CloseHandle(handle);
		}
	}
	Win32Mutex() {
	}
	Win32Mutex(auto_init)
			: Win32Mutex() {
		init();
	}

	void init() {
		ASSERT(!isValid()) << "Trying to initialize mutex more than once";
		handle = CreateMutex(nullptr, FALSE, nullptr);
		ASSERT(handle != INVALID_HANDLE_VALUE) << "Failed to create mutex, error: " << GetLastError();
	}
	void destroy() {
		ASSERT(isValid()) << "mutex is not valid";
		CloseHandle(handle);
		handle = INVALID_HANDLE_VALUE;
	}

	bool isValid() const {
		return handle != INVALID_HANDLE_VALUE;
	}

	void lock() {
		ASSERT(isValid()) << "mutex is not valid";
		DWORD res = WaitForSingleObject(handle, INFINITE);
		ASSERT(res == WAIT_OBJECT_0) << "WaitForSingleObject returned invalid value: " << res << ", error: " << GetLastError();
	}
	void unlock() {
		ASSERT(isValid()) << "mutex is not valid";
		BOOL success = ReleaseMutex(handle);
		ASSERT(success) << "Failed to release mutex, error: " << GetLastError();
	}
};

}

#endif /* WIN32MUTEX_H_ */
