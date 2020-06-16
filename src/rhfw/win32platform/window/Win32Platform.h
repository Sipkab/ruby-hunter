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
#ifndef WIN32PLATFORM_H_
#define WIN32PLATFORM_H_

#include <win32platform/minwindows.h>

namespace rhfw {
namespace win32platform {

HINSTANCE getApplicationInstance();
DWORD getMainThreadId();
LONGLONG getMicroSeconds();

} // namespace win32
} // namespace rhfw

#endif /* WIN32PLATFORM_H_ */
