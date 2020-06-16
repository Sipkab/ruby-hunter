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

#ifndef WINSTORESEMAPHORE_H_
#define WINSTORESEMAPHORE_H_

#include <Windows.h>

#include <gen/configuration.h>
#include <gen/log.h>

namespace rhfw {

class WinstoreSemaphore {
public:
	class auto_init {
	};
private:
	HANDLE handle = INVALID_HANDLE_VALUE;
public:
	WinstoreSemaphore() {
	}
	WinstoreSemaphore(auto_init)
			: WinstoreSemaphore() {
		init();
	}
	~WinstoreSemaphore() {
		if (isValid()) {
			CloseHandle(handle);
		}
	}

	void init() {
		ASSERT(!isValid()) << "Trying to initialize sempahore more than once";
		handle = CreateEvent(nullptr, FALSE, FALSE, nullptr);
		ASSERT(handle != INVALID_HANDLE_VALUE) << "Failed to create semaphore, error: " << GetLastError();
	}
	void destroy() {
		ASSERT(isValid()) << "semaphore is not valid";
		CloseHandle(handle);
		handle = INVALID_HANDLE_VALUE;
	}
	void post() {
		ASSERT(isValid()) << "semaphore is not valid";
		BOOL success = SetEvent(handle);
		ASSERT(success) << "Failed to post semaphore, error: " << GetLastError();
	}
	void wait() {
		ASSERT(isValid()) << "semaphore is not valid";
		DWORD res = WaitForSingleObject(handle, INFINITE);
		ASSERT(res == WAIT_OBJECT_0) << "WaitForSingleObject returned invalid value: " << res << ", error: " << GetLastError();
	}
	bool isValid() const {
		return handle != INVALID_HANDLE_VALUE;
	}

};

}

#endif /* WINSTORESEMAPHORE_H_ */
