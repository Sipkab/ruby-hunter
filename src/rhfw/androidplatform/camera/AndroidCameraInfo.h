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
 * AndroidCameraInfo.h
 *
 *  Created on: 2016. dec. 9.
 *      Author: sipka
 */

#ifndef ANDROIDPLATFORM_CAMERA_ANDROIDCAMERAINFO_H_
#define ANDROIDPLATFORM_CAMERA_ANDROIDCAMERAINFO_H_

#include <framework/io/camera/CameraInfo.h>
#include <gen/fwd/types.h>

namespace rhfw {

class AndroidCameraInfo: public CameraInfoBase {
private:
public:
	unsigned int androidCameraId;
	int cameraDegrees;
};

}  // namespace rhfw

#endif /* ANDROIDPLATFORM_CAMERA_ANDROIDCAMERAINFO_H_ */
