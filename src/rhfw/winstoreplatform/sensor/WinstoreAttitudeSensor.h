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
 * WinstoreAttitudeSensor.h
 *
 *  Created on: 2016. dec. 10.
 *      Author: sipka
 */

#ifndef WINSTOREPLATFORM_SENSOR_WINSTOREATTITUDESENSOR_H_
#define WINSTOREPLATFORM_SENSOR_WINSTOREATTITUDESENSOR_H_

#include <framework/io/sensor/AttitudeSensor.h>
#include <framework/geometry/Matrix.h>
#include <winstoreplatform/sensor/WinstoreSensor.h>

namespace rhfw {

class WinstoreAttitudeSensor: public WinstoreSensor<WinstoreAttitudeSensor>, public AttitudeSensorBase {
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

#endif /* WINSTOREPLATFORM_SENSOR_WINSTOREATTITUDESENSOR_H_ */
