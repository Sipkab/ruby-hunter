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
 * AndroidAttitudeSensor.h
 *
 *  Created on: 2016. dec. 3.
 *      Author: sipka
 */

#ifndef ANDROIDPLATFORM_SENSOR_ANDROIDATTITUDESENSOR_H_
#define ANDROIDPLATFORM_SENSOR_ANDROIDATTITUDESENSOR_H_

#include <androidplatform/appglue.h>
#include <androidplatform/sensor/AndroidSensor.h>
#include <framework/io/sensor/AttitudeSensor.h>
#include <framework/geometry/Matrix.h>

#include <gen/log.h>
#include <gen/fwd/types.h>

namespace rhfw {

class AndroidAttitudeSensor: public AndroidSensor<AndroidAttitudeSensor>,
		public AttitudeSensorBase,
		private androidplatform::AndroidDeviceOrientationChangedListener {
private:
	UIRotation deviceRotation;

	virtual void onAndroidDeviceOrientationChanged(UIRotation rotation) override;
public:
	class Data {
	public:
		UIRotation deviceRotation;
		Vector4F rotationVector;

		Matrix3D getRotationMatrix() const;
		Matrix3D getRotationMatrixWithOrientation() const;
	};

	void updated(Data& data);

	void enableSensor();
	void disableSensor();

	using AndroidSensor::AndroidSensor;
};

}  // namespace rhfw

#endif /* ANDROIDPLATFORM_SENSOR_ANDROIDATTITUDESENSOR_H_ */
