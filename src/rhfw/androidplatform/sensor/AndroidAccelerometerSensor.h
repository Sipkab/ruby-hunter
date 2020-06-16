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
 * AndroidAccelerometerSensor.h
 *
 *  Created on: 2016. dec. 3.
 *      Author: sipka
 */

#ifndef ANDROIDPLATFORM_SENSOR_ANDROIDACCELEROMETERSENSOR_H_
#define ANDROIDPLATFORM_SENSOR_ANDROIDACCELEROMETERSENSOR_H_

#include <framework/io/sensor/AccelerometerSensor.h>
#include <framework/geometry/Vector.h>
#include <androidplatform/sensor/AndroidSensor.h>

namespace rhfw {

class AndroidAccelerometerSensor: public AndroidSensor<AndroidAccelerometerSensor>, public AccelerometerSensorBase {
private:
public:
	class Data {
	public:
		Vector3F acceleration;
	};

	using AndroidSensor::AndroidSensor;
};

}  // namespace rhfw

#endif /* ANDROIDPLATFORM_SENSOR_ANDROIDACCELEROMETERSENSOR_H_ */
