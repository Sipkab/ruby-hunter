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
 * AndroidCameraTextureInputSource.h
 *
 *  Created on: 2016. dec. 2.
 *      Author: sipka
 */

#ifndef ANDROIDPLATFORM_CAMERA_ANDROIDCAMERATEXTUREINPUTSOURCE_H_
#define ANDROIDPLATFORM_CAMERA_ANDROIDCAMERATEXTUREINPUTSOURCE_H_

#include <androidplatform/appglue.h>
#include <framework/io/camera/CameraTextureInputSource.h>
#include <framework/io/camera/Camera.h>
#include <framework/geometry/Matrix.h>

#include <gen/fwd/types.h>

#include <jni.h>

namespace rhfw {
class OpenGlEs20Texture;

class AndroidCameraTextureInputSource: public CameraTextureInputSourceBase,
		private androidplatform::AndroidDeviceOrientationChangedListener,
		private Camera::AndroidCameraDestroyingListener {
private:
	Camera* camera = nullptr;
	UIRotation deviceRotation;

	jobject surfaceTexture = 0;
	jclass surfaceTextureClass = 0;
	jfloatArray matrixArray = 0;
	Matrix3D matrixValue;
	OpenGlEs20Texture* texture = nullptr;

	void releaseResources();
private:
	virtual void onAndroidDeviceOrientationChanged(UIRotation rotation) override;

	virtual void onCameraDestroying() override;
public:
	AndroidCameraTextureInputSource(Camera* camera);
	~AndroidCameraTextureInputSource();

	virtual void apply(render::Texture* texture) override;
	virtual void apply(OpenGlEs20Texture* texture) override;

	virtual void startPreview() override;
	virtual void stopPreview() override;

	void updateTexture();

	OpenGlEs20Texture* getTexture() {
		return texture;
	}

	Matrix3D getTransformationMatrix() const;
};

} // namespace rhfw

#endif /* ANDROIDPLATFORM_CAMERA_ANDROIDCAMERATEXTUREINPUTSOURCE_H_ */
