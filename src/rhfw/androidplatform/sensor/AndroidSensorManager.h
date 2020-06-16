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
 * AndroidSensorManager.h
 *
 *  Created on: 2016. dec. 3.
 *      Author: sipka
 */

#ifndef ANDROIDPLATFORM_SENSOR_ANDROIDSENSORMANAGER_H_
#define ANDROIDPLATFORM_SENSOR_ANDROIDSENSORMANAGER_H_

#include <framework/io/sensor/SensorManager.h>

#include <androidplatform/sensor/AndroidAccelerometerSensor.h>
#include <androidplatform/sensor/AndroidAttitudeSensor.h>

#include <android/sensor.h>

namespace rhfw {

class AndroidSensorManager: public SensorManagerBase {
private:
	static int sensorLooperCallback(int fd, int events, void* unused);

	ASensorManager* androidSensorManager = nullptr;
	ASensorEventQueue* eventQueue = nullptr;

	AndroidAccelerometerSensor* accelerometer = nullptr;
	AndroidAttitudeSensor* attitude = nullptr;

public:
	virtual bool load() override;
	virtual void free() override;

	AndroidSensorManager();
	~AndroidSensorManager();

	ASensorManager* getAndroidSensorManager() {
		return androidSensorManager;
	}
	ASensorEventQueue* getAndroidEventQueue() {
		return eventQueue;
	}

	virtual AndroidAccelerometerSensor* getAccelerometerSensor() override;
	virtual AndroidAttitudeSensor* getAttitudeSensor() override;
};

} // namespace rhfw

#endif /* ANDROIDPLATFORM_SENSOR_ANDROIDSENSORMANAGER_H_ */
