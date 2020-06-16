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
 * Win32AccelerometerSensor.h
 *
 *  Created on: 2016. dec. 10.
 *      Author: sipka
 */

#ifndef WIN32PLATFORM_SENSOR_WIN32ACCELEROMETERSENSOR_H_
#define WIN32PLATFORM_SENSOR_WIN32ACCELEROMETERSENSOR_H_

#include <framework/io/sensor/AccelerometerSensor.h>
#include <win32platform/sensor/Win32Sensor.h>

namespace rhfw {

class Win32AccelerometerSensor: public Win32Sensor<Win32AccelerometerSensor>, public AccelerometerSensorBase {
private:
public:
	class Data {
	public:
		Vector3F acceleration;
	};
};

}  // namespace rhfw

#endif /* WIN32PLATFORM_SENSOR_WIN32ACCELEROMETERSENSOR_H_ */
