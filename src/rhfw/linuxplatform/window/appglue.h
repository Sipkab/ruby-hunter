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
 * appglue.h
 *
 *  Created on: 2016. szept. 1.
 *      Author: sipka
 */

#ifndef LINUXPLATFORM_APPGLUE_H_
#define LINUXPLATFORM_APPGLUE_H_

#include <linuxplatform/window/glxfunctions.h>
#include <linuxplatform/window/x11functions.h>

#define X11_WINDOW_EVENT_MASKS (ExposureMask | KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask | PointerMotionMask | ButtonMotionMask | StructureNotifyMask | SubstructureNotifyMask | VisibilityChangeMask)

namespace rhfw {
namespace core {

class native_window_internal {
public:
	::Window window = 0;
	x11server::VisualInfoTracker visualInfo;
	Colormap colorMap = 0;
	Cursor cursor = None;
};

}  // namespace core
}  // namespace rhfw

#endif /* LINUXPLATFORM_APPGLUE_H_ */
