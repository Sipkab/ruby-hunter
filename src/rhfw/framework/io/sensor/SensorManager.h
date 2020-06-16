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
 * SensorManager.h
 *
 *  Created on: 2016. dec. 3.
 *      Author: sipka
 */

#ifndef FRAMEWORK_IO_SENSOR_SENSORMANAGER_H_
#define FRAMEWORK_IO_SENSOR_SENSORMANAGER_H_

#include <framework/resource/ShareableResource.h>
#include <framework/io/sensor/Sensor.h>
#include <framework/io/sensor/AttitudeSensor.h>
#include <framework/io/sensor/AccelerometerSensor.h>

#include <gen/platform.h>

namespace rhfw {

class AccelerometerSensorBase;
class AttitudeSensorBase;

class SensorManagerBase: public ShareableResource {
public:
	virtual bool load() override = 0;
	virtual void free() override = 0;
	virtual bool reload() override;

	SensorManagerBase();
	virtual ~SensorManagerBase();

	virtual AccelerometerSensor* getAccelerometerSensor() {
		return nullptr;
	}
	virtual AttitudeSensor* getAttitudeSensor() {
		return nullptr;
	}
};

} // namespace rhfw

#include SENSORMANAGER_EXACT_CLASS_INCLUDE
namespace rhfw {
typedef class SENSORMANAGER_EXACT_CLASS_TYPE SensorManager;
} // namespace rhfw

#endif /* FRAMEWORK_IO_SENSOR_SENSORMANAGER_H_ */
