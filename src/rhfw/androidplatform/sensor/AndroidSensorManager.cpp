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
 * AndroidSensorManager.cpp
 *
 *  Created on: 2016. dec. 3.
 *      Author: sipka
 */

#include <androidplatform/sensor/AndroidSensorManager.h>

#include <gen/log.h>

/**
 * These are not defined in the android/sensor.h, however still are available
 */
#define SENSOR_TYPE_ROTATION_VECTOR 11

namespace rhfw {

int AndroidSensorManager::sensorLooperCallback(int fd, int events, void* unused) {
	AndroidSensorManager* thiz = (AndroidSensorManager*) unused;

	ASensorEvent aevents[16];
	for (ssize_t res; (res = ASensorEventQueue_getEvents(thiz->eventQueue, aevents, 16)) > 0;) {
		for (int i = 0; i < res; ++i) {
			auto& event = aevents[i];
			switch (event.type) {
				case ASENSOR_TYPE_ACCELEROMETER: {
					AccelerometerSensor::Data value;
					value.acceleration = Vector3F { event.acceleration.x, event.acceleration.y, event.acceleration.z };
					thiz->accelerometer->valueChanged(value);
					break;
				}
				case SENSOR_TYPE_ROTATION_VECTOR: {
					AttitudeSensor::Data value;
					value.rotationVector = Vector4F { event.data[0], event.data[1], event.data[2], event.data[3] };
					thiz->attitude->updated(value);
					thiz->attitude->valueChanged(value);
					break;
				}
				default: {
					break;
				}
			}
		}
	}

	return 1;
}

AndroidSensorManager::AndroidSensorManager() {
}
AndroidSensorManager::~AndroidSensorManager() {
	ASSERT(androidSensorManager == nullptr) << "SensorManager was not freed";
}

bool AndroidSensorManager::load() {
	androidSensorManager = ASensorManager_getInstance();
	eventQueue = ASensorManager_createEventQueue(androidSensorManager, ALooper_forThread(), ALOOPER_POLL_CALLBACK, sensorLooperCallback,
			this);
	return true;
}
void AndroidSensorManager::free() {
	if (accelerometer != nullptr) {
		delete accelerometer;
		accelerometer = nullptr;
	}
	if (attitude != nullptr) {
		delete attitude;
		attitude = nullptr;
	}
	ASensorManager_destroyEventQueue(androidSensorManager, eventQueue);
	androidSensorManager = nullptr;
	eventQueue = nullptr;
}

AndroidAccelerometerSensor* AndroidSensorManager::getAccelerometerSensor() {
	if (accelerometer == nullptr) {
		auto* sensor = ASensorManager_getDefaultSensor(androidSensorManager, ASENSOR_TYPE_ACCELEROMETER);
		if (sensor == nullptr) {
			return nullptr;
		}
		accelerometer = new AndroidAccelerometerSensor(eventQueue, sensor);
	}
	return accelerometer;
}

AndroidAttitudeSensor* AndroidSensorManager::getAttitudeSensor() {
	if (attitude == nullptr) {
		auto* sensor = ASensorManager_getDefaultSensor(androidSensorManager, SENSOR_TYPE_ROTATION_VECTOR);
		if (sensor == nullptr) {
			return nullptr;
		}
		attitude = new AndroidAttitudeSensor(eventQueue, sensor);
	}
	return attitude;
}

} // namespace rhfw
