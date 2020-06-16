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
 * AndroidCameraManager.cpp
 *
 *  Created on: 2016. dec. 9.
 *      Author: sipka
 */

#include <androidplatform/camera/AndroidCameraManager.h>
#include <framework/threading/Thread.h>

#include <gen/types.h>

/**
 * From android.camera.Camera$CameraInfo
 */
#define ANDROID_CAMERA_FACING_BACK 0
#define ANDROID_CAMERA_FACING_FRONT 1

namespace rhfw {

AndroidCameraManager::AndroidCameraManager() {
}
AndroidCameraManager::~AndroidCameraManager() {
}

Camera* AndroidCameraManager::openCameraImpl(const CameraInfo& info) {
	JNIEnv& env = *Thread::getJniEnvForCurrentThread();
	static jmethodID openCameraMethod = env.GetStaticMethodID(cameraJavaClass, "open", "(I)Landroid/hardware/Camera;");
	jobject result = env.CallStaticObjectMethod(cameraJavaClass, openCameraMethod, info.androidCameraId);
	if (env.ExceptionCheck()) {
		env.ExceptionClear();
		if (result != 0) {
			env.DeleteLocalRef(result);
		}
		return nullptr;
	}
	if (result == 0) {
		return nullptr;
	}
	jobject cameraobj = env.NewGlobalRef(result);
	env.DeleteLocalRef(result);
	return new Camera(info, cameraobj, this);
}

bool AndroidCameraManager::loadImpl() {
	JNIEnv& env = *Thread::getJniEnvForCurrentThread();
	{
		jclass camclass = env.FindClass("android/hardware/Camera");
		cameraJavaClass = (jclass) env.NewGlobalRef(camclass);
		env.DeleteLocalRef(camclass);
	}

	static jmethodID getNumberOfCamerasMethod = env.GetStaticMethodID(cameraJavaClass, "getNumberOfCameras", "()I");

	cameraCount = env.CallStaticIntMethod(cameraJavaClass, getNumberOfCamerasMethod);

	if (cameraCount == 0) {
		return true;
	}

	cameraInfos = new CameraInfo[cameraCount];

	static jmethodID getCameraInfoMethod = env.GetStaticMethodID(cameraJavaClass, "getCameraInfo",
			"(ILandroid/hardware/Camera$CameraInfo;)V");
	static jmethodID openMethod = env.GetStaticMethodID(cameraJavaClass, "open", "(I)Landroid/hardware/Camera;");
	jclass infoclass = env.FindClass("android/hardware/Camera$CameraInfo");

	static jmethodID infoInitMethod = env.GetMethodID(infoclass, "<init>", "()V");
	static jfieldID infoFacingField = env.GetFieldID(infoclass, "facing", "I");
	static jfieldID infoOrientationField = env.GetFieldID(infoclass, "orientation", "I");

	jobject camerainfoobject = env.NewObject(infoclass, infoInitMethod);
	env.DeleteLocalRef(infoclass);

	for (unsigned int i = 0; i < cameraCount; ++i) {
		cameraInfos[i].androidCameraId = i;
		env.CallStaticVoidMethod(cameraJavaClass, getCameraInfoMethod, i, camerainfoobject);
		jint facing = env.GetIntField(camerainfoobject, infoFacingField);
		jint orientation = env.GetIntField(camerainfoobject, infoOrientationField);
		cameraInfos[i].facing = facing == ANDROID_CAMERA_FACING_FRONT ? CameraFacing::FRONT : CameraFacing::BACK;
		cameraInfos[i].cameraDegrees = orientation;
	}

	env.DeleteLocalRef(camerainfoobject);
	return true;
}

void AndroidCameraManager::freeImpl() {
	JNIEnv& env = *Thread::getJniEnvForCurrentThread();
	env.DeleteGlobalRef(cameraJavaClass);

	cameraJavaClass = nullptr;

	delete[] cameraInfos;
	cameraCount = 0;
}

} // namespace rhfw
