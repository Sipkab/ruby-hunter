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
 * MacOsxAttitudeSensor.h
 *
 *  Created on: 2016. dec. 10.
 *      Author: sipka
 */

#ifndef MACOSXPLATFORM_SENSOR_MACOSXATTITUDESENSOR_H_
#define MACOSXPLATFORM_SENSOR_MACOSXATTITUDESENSOR_H_

#include <framework/io/sensor/AttitudeSensor.h>
#include <framework/geometry/Matrix.h>
#include <macosxplatform/sensor/MacOsxSensor.h>

namespace rhfw {

class MacOsxAttitudeSensor: public MacOsxSensor<MacOsxAttitudeSensor>, public AttitudeSensorBase {
private:
public:
	class Data {
	public:
		Matrix3D getRotationMatrix() const {
			return Matrix3D { }.setIdentity();
		}
		Matrix3D getRotationMatrixWithOrientation() const {
			return getRotationMatrix();
		}
	};
};

}
// namespace rhfw

#endif /* MACOSXPLATFORM_SENSOR_MACOSXATTITUDESENSOR_H_ */
