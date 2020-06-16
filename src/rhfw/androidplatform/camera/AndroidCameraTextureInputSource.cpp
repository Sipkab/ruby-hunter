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
 * AndroidCameraTextureInputSource.cpp
 *
 *  Created on: 2016. dec. 2.
 *      Author: sipka
 */

#include <androidplatform/appglue.h>
#include <androidplatform/camera/AndroidCameraTextureInputSource.h>
#include <androidplatform/AndroidPlatform.h>

#include <opengles20/texture/OpenGlEs20Texture.h>
#include <framework/threading/Thread.h>
#include <framework/io/camera/CameraManager.h>

#include <gen/log.h>

namespace rhfw {

static jmethodID setPreviewTextureMethod = 0;

AndroidCameraTextureInputSource::AndroidCameraTextureInputSource(Camera* camera)
		: camera(camera) {
	camera->androidCameraDestroyingEvents += *this;
}
AndroidCameraTextureInputSource::~AndroidCameraTextureInputSource() {
	if (surfaceTexture != nullptr) {
		releaseResources();
	}
}

void AndroidCameraTextureInputSource::apply(render::Texture* texture) {
	THROW()<< "OpenGlEs20Texture method shouldve been called";
}

void AndroidCameraTextureInputSource::apply(OpenGlEs20Texture* texture) {
	if (androidplatform::getSdkVersion() < 11) {
		asInitializer(texture)->initFailed();
		return;
	}
	if (camera == nullptr) {
		asInitializer(texture)->initFailed();
		return;
	}
	this->texture = texture;
	JNIEnv& env = *Thread::getJniEnvForCurrentThread();
	{
		jfloatArray locarr = env.NewFloatArray(16);
		matrixArray = (jfloatArray) env.NewGlobalRef(locarr);
		env.DeleteLocalRef(locarr);
	}

	{
		jclass surfacetextureclass = env.FindClass("android/graphics/SurfaceTexture");
		surfaceTextureClass = (jclass) env.NewGlobalRef(surfacetextureclass);
		env.DeleteLocalRef(surfacetextureclass);
	}
	static jmethodID surfaceTextureInitMethod = env.GetMethodID(surfaceTextureClass, "<init>", "(I)V");

	{
		auto glname = texture->getGlName();
		jobject re = env.NewObject(surfaceTextureClass, surfaceTextureInitMethod, glname);
		surfaceTexture = env.NewGlobalRef(re);
		env.DeleteLocalRef(re);
	}

	if (!setPreviewTextureMethod) {
		setPreviewTextureMethod = env.GetMethodID(camera->getCameraManager()->getJavaCameraClass(), "setPreviewTexture",
				"(Landroid/graphics/SurfaceTexture;)V");
	}
	env.CallVoidMethod(camera->getJavaCameraObject(), setPreviewTextureMethod, surfaceTexture);
	if (env.ExceptionCheck()) {
		env.ExceptionClear();
		//failed to set preview texture;
		env.DeleteGlobalRef(surfaceTexture);
		env.DeleteGlobalRef(matrixArray);
		env.DeleteGlobalRef(surfaceTextureClass);

		surfaceTexture = nullptr;
		asInitializer(texture)->initFailed();
		return;
	}

	texture->initGlTextureTarget(GL_TEXTURE_EXTERNAL_OES);
}

void AndroidCameraTextureInputSource::updateTexture() {
	ASSERT(camera != nullptr && surfaceTexture != nullptr);
	if (surfaceTexture == nullptr) {
		return;
	}

	JNIEnv& env = *Thread::getJniEnvForCurrentThread();
	static jmethodID updatemethodid = env.GetMethodID(surfaceTextureClass, "updateTexImage", "()V");
	static jmethodID matrixmethodid = env.GetMethodID(surfaceTextureClass, "getTransformMatrix", "([F)V");

	this->texture->bind();

	env.CallVoidMethod(surfaceTexture, updatemethodid);
	CHECK_GL_ERROR_REND(texture->getRenderer())

	Matrix3D mat;
	env.CallVoidMethod(surfaceTexture, matrixmethodid, matrixArray);
	env.GetFloatArrayRegion(matrixArray, 0, 16, mat);
	mat.transpose();

	matrixValue = mat;

	if (androidplatform::getSdkVersion() < 17) {
		this->deviceRotation = androidplatform::queryDeviceRotation(*Thread::getJniEnvForCurrentThread());
	}
}

void AndroidCameraTextureInputSource::startPreview() {
	ASSERT(camera != nullptr && surfaceTexture != nullptr);
	if (surfaceTexture == nullptr) {
		return;
	}

	if (androidplatform::getSdkVersion() >= 17) {
		deviceRotation = androidplatform::subscribeDeviceOrientationChange(this);
	} else {
		deviceRotation = androidplatform::queryDeviceRotation(*Thread::getJniEnvForCurrentThread());
	}

	camera->callStartPreview();
}

void AndroidCameraTextureInputSource::stopPreview() {
	ASSERT(camera != nullptr && surfaceTexture != nullptr);
	if (surfaceTexture == nullptr) {
		return;
	}

	if (androidplatform::getSdkVersion() >= 17) {
		androidplatform::unsubscribeDeviceOrientationChange(this);
	}
	camera->callStopPreview();
}

Matrix3D AndroidCameraTextureInputSource::getTransformationMatrix() const {
	int rotdegrees = (int) deviceRotation * (int) UIRotation::ROTATION_DEGREE;
	int degrees;

	if (camera->getInfo().facing == CameraFacing::FRONT) {
		// compensate the mirror as in documentation
		degrees = (camera->getInfo().cameraDegrees + rotdegrees) % 360;
		degrees = (360 - degrees) % 360;
	} else {
		// back-facing
		degrees = camera->getInfo().cameraDegrees - rotdegrees;
	}
	return Matrix3D { }.setTranslate(-0.5f, -0.5f, 0.0f).multFlipY().multRotate(M_PI * -degrees / 180.0f, 0, 0, 1).multTranslate(0.5f, 0.5f,
			0.0f) *= matrixValue;
}

void AndroidCameraTextureInputSource::onAndroidDeviceOrientationChanged(UIRotation rotation) {
	this->deviceRotation = rotation;
}

void AndroidCameraTextureInputSource::releaseResources() {
	ASSERT(surfaceTexture != nullptr);

	JNIEnv& env = *Thread::getJniEnvForCurrentThread();

	env.CallVoidMethod(camera->getJavaCameraObject(), setPreviewTextureMethod, nullptr);

	if (androidplatform::getSdkVersion() >= 14) {
		static jmethodID releaseMethod = env.GetMethodID(surfaceTextureClass, "release", "()V");
		env.CallVoidMethod(surfaceTexture, releaseMethod);
	}

	env.DeleteGlobalRef(surfaceTextureClass);
	env.DeleteGlobalRef(surfaceTexture);
	env.DeleteGlobalRef(matrixArray);

	surfaceTexture = nullptr;
}

void AndroidCameraTextureInputSource::onCameraDestroying() {
	if (camera != nullptr) {
		if (surfaceTexture != nullptr) {
			releaseResources();
		}
		camera = nullptr;
	}
}

} // namespace rhfw

