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
#ifndef WINDOWS_EXCEPTION_HELPER_H_
#define WINDOWS_EXCEPTION_HELPER_H_

#include <gen/configuration.h>
#include <gen/log.h>

#if LOGGING_ENABLED
#include <Windows.h>
#include <gen/log.h>
#define ThrowIfFailed(res) \
{ \
	HRESULT hr = (res); \
	if (FAILED(hr)) { \
		LOGTRACE() << "HRESULT: "  << hr; \
		throw Platform::Exception::CreateException(hr); \
	} \
}
#else
//this should not even be used in release builds
#define ThrowIfFailed(res) (res)
#endif /* DEBUG */

#endif /* WINDOWS_EXCEPTION_HELPER_H_ */
