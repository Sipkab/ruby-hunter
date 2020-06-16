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
 * AttitudeSensor.h
 *
 *  Created on: 2016. dec. 3.
 *      Author: sipka
 */

#ifndef FRAMEWORK_IO_SENSOR_ATTITUDESENSOR_H_
#define FRAMEWORK_IO_SENSOR_ATTITUDESENSOR_H_

#include <framework/io/sensor/Sensor.h>

#include <gen/platform.h>

namespace rhfw {

class AttitudeSensorBase: public SensorBase {
private:
public:
};

}  // namespace rhfw

#include ATTITUDESENSOR_EXACT_CLASS_INCLUDE
namespace rhfw {
typedef class ATTITUDESENSOR_EXACT_CLASS_TYPE AttitudeSensor;
} // namespace rhfw

#endif /* FRAMEWORK_IO_SENSOR_ATTITUDESENSOR_H_ */
