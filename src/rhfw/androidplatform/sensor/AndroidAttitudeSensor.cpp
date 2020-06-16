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
 * AndroidAttitudeSensor.cpp
 *
 *  Created on: 2016. dec. 7.
 *      Author: sipka
 */

#include <androidplatform/appglue.h>
#include <framework/io/sensor/AttitudeSensor.h>
#include <framework/threading/Thread.h>

#include <androidplatform/AndroidPlatform.h>

namespace rhfw {

Matrix3D AndroidAttitudeSensor::Data::getRotationMatrix() const {
	Matrix3D result;
	float q0 = rotationVector.w();
	float q1 = rotationVector.x();
	float q2 = rotationVector.y();
	float q3 = rotationVector.z();

	float sq_q1 = 2 * q1 * q1;
	float sq_q2 = 2 * q2 * q2;
	float sq_q3 = 2 * q3 * q3;
	float q1_q2 = 2 * q1 * q2;
	float q3_q0 = 2 * q3 * q0;
	float q1_q3 = 2 * q1 * q3;
	float q2_q0 = 2 * q2 * q0;
	float q2_q3 = 2 * q2 * q3;
	float q1_q0 = 2 * q1 * q0;

	result[0] = 1 - sq_q2 - sq_q3;
	result[1] = q1_q2 - q3_q0;
	result[2] = q1_q3 + q2_q0;
	result[3] = 0.0f;

	result[4] = q1_q2 + q3_q0;
	result[5] = 1 - sq_q1 - sq_q3;
	result[6] = q2_q3 - q1_q0;
	result[7] = 0.0f;

	result[8] = q1_q3 - q2_q0;
	result[9] = q2_q3 + q1_q0;
	result[10] = 1 - sq_q1 - sq_q2;
	result[11] = 0.0f;

	result[12] = result[13] = result[14] = 0.0f;
	result[15] = 1.0f;

	result.transpose();
	return result;
}

Matrix3D AndroidAttitudeSensor::Data::getRotationMatrixWithOrientation() const {
	Matrix3D result = getRotationMatrix();
	int rotdegrees = (int) deviceRotation * (int) UIRotation::ROTATION_DEGREE;
	result.multRotate(M_PI * rotdegrees / 180.0f, 0, 0, -1);
	return result;
}

void AndroidAttitudeSensor::enableSensor() {
	AndroidSensor<AndroidAttitudeSensor>::enableSensor();
	if (androidplatform::getSdkVersion() >= 17) {
		deviceRotation = androidplatform::subscribeDeviceOrientationChange(this);
	} else {
		deviceRotation = androidplatform::queryDeviceRotation(*Thread::getJniEnvForCurrentThread());
	}
}

void AndroidAttitudeSensor::disableSensor() {
	AndroidSensor<AndroidAttitudeSensor>::disableSensor();
	if (androidplatform::getSdkVersion() >= 17) {
		androidplatform::unsubscribeDeviceOrientationChange(this);
	}
}

void AndroidAttitudeSensor::updated(Data& data) {
	if (androidplatform::getSdkVersion() < 17) {
		data.deviceRotation = androidplatform::queryDeviceRotation(*Thread::getJniEnvForCurrentThread());
	} else {
		data.deviceRotation = this->deviceRotation;
	}
}

void AndroidAttitudeSensor::onAndroidDeviceOrientationChanged(UIRotation rotation) {
	this->deviceRotation = rotation;
}

}  // namespace rhfw

