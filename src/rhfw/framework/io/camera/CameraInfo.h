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
 * CameraInfo.h
 *
 *  Created on: 2016. dec. 9.
 *      Author: sipka
 */

#ifndef FRAMEWORK_IO_CAMERA_CAMERAINFO_H_
#define FRAMEWORK_IO_CAMERA_CAMERAINFO_H_

#include <gen/fwd/types.h>
#include <gen/platform.h>

namespace rhfw {

class CameraInfoBase {
public:
	CameraFacing facing;
};

}  // namespace rhfw

#include CAMERAINFO_EXACT_CLASS_INCLUDE
namespace rhfw {
typedef class CAMERAINFO_EXACT_CLASS_TYPE CameraInfo;
} // namespace rhfw

#endif /* FRAMEWORK_IO_CAMERA_CAMERAINFO_H_ */
