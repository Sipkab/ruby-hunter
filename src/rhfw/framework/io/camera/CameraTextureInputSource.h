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
 * CameraTextureInputSource.h
 *
 *  Created on: 2016. dec. 2.
 *      Author: sipka
 */

#ifndef FRAMEWORK_IO_CAMERA_CAMERATEXTUREINPUTSOURCE_H_
#define FRAMEWORK_IO_CAMERA_CAMERATEXTUREINPUTSOURCE_H_

#include <framework/render/Texture.h>

#include <gen/platform.h>

namespace rhfw {

class CameraTextureInputSourceBase: public render::TextureInputSource {
public:
	virtual void startPreview() = 0;
	virtual void stopPreview() = 0;
};

} // namespace rhfw

#include CAMERATEXTUREINPUTSOURCE_EXACT_CLASS_INCLUDE
namespace rhfw {
typedef class CAMERATEXTUREINPUTSOURCE_EXACT_CLASS_TYPE CameraTextureInputSource;
} // namespace rhfw

#endif /* FRAMEWORK_IO_CAMERA_CAMERATEXTUREINPUTSOURCE_H_ */
