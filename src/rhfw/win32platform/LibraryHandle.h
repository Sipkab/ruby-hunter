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
 * LibraryHandle.h
 *
 *  Created on: 2016. szept. 10.
 *      Author: sipka
 */

#ifndef WIN32PLATFORM_LIBRARYHANDLE_H_
#define WIN32PLATFORM_LIBRARYHANDLE_H_

#include <win32platform/minwindows.h>
#include <gen/log.h>
#include <gen/fwd/types.h>

namespace rhfw {

class LibraryHandle {
private:
	HMODULE module = NULL;
public:
	explicit LibraryHandle(HMODULE module = NULL)
			: module(module) {
	}
	explicit LibraryHandle(LPCTSTR modulename)
			: module(LoadLibraryA(modulename)) {
		WARN(module == NULL) << "Failed to load module: " << modulename;
	}
	LibraryHandle(LibraryHandle&& o)
			: module(o.module) {
		o.module = NULL;
	}
	LibraryHandle(const LibraryHandle&) = delete;
	LibraryHandle& operator=(LibraryHandle&& o) {
		if (module != NULL) {
			FreeLibrary(module);
		}

		this->module = o.module;
		o.module = NULL;
		return *this;
	}
	LibraryHandle& operator=(const LibraryHandle&) = delete;
	LibraryHandle& operator=(NULLPTR_TYPE) {
		if (module != NULL) {
			FreeLibrary(module);
			module = NULL;
		}
		return *this;
	}
	~LibraryHandle() {
		if (module != NULL) {
			FreeLibrary(module);
		}
	}
	void free() {
		if (module != NULL) {
			FreeLibrary(module);
			module = NULL;
		}
	}
	bool load(LPCTSTR modulename) {
		module = LoadLibraryA(modulename);
		WARN(module == NULL) << "Failed to load module: " << modulename;
		return module != NULL;
	}

	operator bool() const {
		return module != NULL;
	}
	bool operator!() const {
		return module == NULL;
	}
	explicit operator HMODULE() const {
		return module;
	}
};

}  // namespace rhfw

#endif /* WIN32PLATFORM_LIBRARYHANDLE_H_ */
