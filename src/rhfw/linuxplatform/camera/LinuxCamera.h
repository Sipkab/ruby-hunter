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
 * LinuxCamera.h
 *
 *  Created on: 2016. dec. 10.
 *      Author: sipka
 */

#ifndef LINUXPLATFORM_CAMERA_LINUXCAMERA_H_
#define LINUXPLATFORM_CAMERA_LINUXCAMERA_H_

#include <framework/io/camera/Camera.h>

namespace rhfw {

class LinuxCamera: public CameraBase {
private:
public:
	LinuxCamera(const CameraInfo& info)
			: CameraBase(info) {
	}
};

}  // namespace rhfw

#endif /* LINUXPLATFORM_CAMERA_LINUXCAMERA_H_ */
