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
 * AndroidCamera.cpp
 *
 *  Created on: 2016. dec. 2.
 *      Author: sipka
 */

#include <framework/io/camera/Camera.h>
#include <framework/io/camera/CameraManager.h>
#include <framework/threading/Thread.h>

#include <gen/log.h>

namespace rhfw {

AndroidCamera::AndroidCamera(const CameraInfo& camerainfo, jobject cameraobject, AndroidCameraManager* manager)
		: CameraBase(camerainfo), manager(manager), cameraObject(cameraobject) {
}
AndroidCamera::~AndroidCamera() {
	ASSERT(!previewRunning);

	for (auto&& l : androidCameraDestroyingEvents.foreach()) {
		l.onCameraDestroying();
	}

	JNIEnv& env = *Thread::getJniEnvForCurrentThread();
	static jmethodID releaseMethod = env.GetMethodID(manager->getJavaCameraClass(), "release", "()V");
	env.CallVoidMethod(cameraObject, releaseMethod);
	env.DeleteGlobalRef(cameraObject);
}

void AndroidCamera::callStartPreview() {
	ASSERT(!previewRunning);

	JNIEnv& env = *Thread::getJniEnvForCurrentThread();
	static jmethodID startPreviewMethod = env.GetMethodID(manager->getJavaCameraClass(), "startPreview", "()V");
	env.CallVoidMethod(cameraObject, startPreviewMethod);
	previewRunning = true;
}

void AndroidCamera::callStopPreview() {
	ASSERT(previewRunning);

	JNIEnv& env = *Thread::getJniEnvForCurrentThread();
	static jmethodID stopPreviewMethod = env.GetMethodID(manager->getJavaCameraClass(), "stopPreview", "()V");
	env.CallVoidMethod(cameraObject, stopPreviewMethod);
	previewRunning = false;
}

} // namespace rhfw
