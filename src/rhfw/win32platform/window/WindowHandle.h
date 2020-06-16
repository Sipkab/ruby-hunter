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
 * WindowHandle.h
 *
 *  Created on: 2016. szept. 14.
 *      Author: sipka
 */

#ifndef WIN32PLATFORM_WINDOWHANDLE_H_
#define WIN32PLATFORM_WINDOWHANDLE_H_

#include <win32platform/minwindows.h>
#include <gen/log.h>

namespace rhfw {

namespace win32platform {

class WindowHandle {
public:
	HWND hwnd;

	explicit WindowHandle(HWND hwnd)
			: hwnd(hwnd) {
	}
	WindowHandle(WindowHandle&& o)
			: hwnd(o.hwnd) {
		o.hwnd = NULL;
	}
	~WindowHandle() {
		if (hwnd != NULL) {
			BOOL res = DestroyWindow(hwnd);
			ASSERT(res) << GetLastError();
		}
	}

	operator HWND() const {
		return hwnd;
	}

	HWND take() {
		HWND result = hwnd;
		this->hwnd = NULL;
		return result;
	}
	operator bool() const {
		return hwnd != NULL;
	}
	bool operator!() const {
		return hwnd == NULL;
	}
};

}  // namespace win32platform

}  // namespace rhfw

#endif /* WIN32PLATFORM_WINDOWHANDLE_H_ */
