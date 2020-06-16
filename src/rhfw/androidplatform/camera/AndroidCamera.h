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
 * AndroidCamera.h
 *
 *  Created on: 2016. dec. 2.
 *      Author: sipka
 */

#ifndef ANDROIDPLATFORM_CAMERA_ANDROIDCAMERA_H_
#define ANDROIDPLATFORM_CAMERA_ANDROIDCAMERA_H_

#include <framework/io/camera/Camera.h>
#include <framework/utils/BasicListener.h>

#include <jni.h>

namespace rhfw {

class AndroidCameraManager;

class AndroidCamera: public CameraBase {
	AndroidCameraManager* manager = nullptr;
	jobject cameraObject;

	bool previewRunning = false;
public:
	class AndroidCameraDestroyingListener: public BasicListener<AndroidCameraDestroyingListener> {
	public:
		virtual void onCameraDestroying() = 0;
	};
	AndroidCameraDestroyingListener::Events androidCameraDestroyingEvents;

	AndroidCamera(const CameraInfo& camerainfo, jobject cameraobject, AndroidCameraManager* manager);
	~AndroidCamera();

	void callStartPreview();
	void callStopPreview();

	AndroidCameraManager* getCameraManager() {
		return manager;
	}

	jobject getJavaCameraObject() {
		return cameraObject;
	}
};

} // namespace rhfw

#endif /* ANDROIDPLATFORM_CAMERA_ANDROIDCAMERA_H_ */
