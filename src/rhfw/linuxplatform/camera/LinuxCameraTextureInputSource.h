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
 * LinuxCameraTextureInputSource.h
 *
 *  Created on: 2016. dec. 10.
 *      Author: sipka
 */

#ifndef LINUXPLATFORM_CAMERA_LINUXCAMERATEXTUREINPUTSOURCE_H_
#define LINUXPLATFORM_CAMERA_LINUXCAMERATEXTUREINPUTSOURCE_H_

#include <framework/io/camera/CameraTextureInputSource.h>
#include <framework/io/camera/Camera.h>
#include <framework/geometry/Matrix.h>

#include <gen/log.h>

namespace rhfw {

class LinuxCameraTextureInputSource: public CameraTextureInputSourceBase {
private:
public:
	LinuxCameraTextureInputSource(Camera* camera) {
	}
	virtual void startPreview() override {
	}
	virtual void stopPreview() override {
	}

	void updateTexture() {
	}
	Matrix3D getTransformationMatrix() {
		return Matrix3D { }.setIdentity();
	}

	virtual void apply(render::Texture* texture) override {
		THROW()<<"unimplemented";
	}
};

}
 // namespace rhfw

#endif /* LINUXPLATFORM_CAMERA_LINUXCAMERATEXTUREINPUTSOURCE_H_ */
