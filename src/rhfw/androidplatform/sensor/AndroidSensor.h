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
 * AndroidSensor.h
 *
 *  Created on: 2016. dec. 3.
 *      Author: sipka
 */

#ifndef ANDROIDPLATFORM_SENSOR_ANDROIDSENSOR_H_
#define ANDROIDPLATFORM_SENSOR_ANDROIDSENSOR_H_

#include <android/sensor.h>

#include <framework/io/sensor/SensorListener.h>
#include <framework/utils/LinkedList.h>

#include <gen/log.h>

namespace rhfw {

class AndroidSensorManager;

template<typename SensorType>
class AndroidSensor {
private:
	friend class SensorListener<SensorType> ;
	friend class AndroidSensorManager;

	ASensorEventQueue* eventQueue = nullptr;
	ASensor const* sensor = nullptr;

	void dispatchEnableSensor() {
		static_cast<SensorType*>(this)->enableSensor();
	}
	void dispatchDisableSensor() {
		static_cast<SensorType*>(this)->disableSensor();
	}
protected:
	void enableSensor() {
		ASSERT(sensor != nullptr);
		ASSERT(eventQueue != nullptr);
		int res = ASensorEventQueue_enableSensor(eventQueue, sensor);
		ASSERT(res >= 0);
		res = ASensorEventQueue_setEventRate(eventQueue, sensor, ASensor_getMinDelay(sensor));
		ASSERT(res >= 0);
	}
	void disableSensor() {
		ASSERT(sensor != nullptr);
		ASSERT(eventQueue != nullptr);
		int res = ASensorEventQueue_disableSensor(eventQueue, sensor);
		ASSERT(res >= 0);
	}

	LinkedList<SensorListener<SensorType>, false> listeners;

	template<typename Value>
	void valueChanged(const Value& value) {
		for (auto&& l : listeners.objects()) {
			l.onSensorChanged(value);
		}
	}

	void subscribe(LinkedNode<SensorListener<SensorType>>* listener) {
		if (this->listeners.isEmpty()) {
			dispatchEnableSensor();
		}
		listeners.addToEnd(*listener);
	}

	void unsubscribe(LinkedNode<SensorListener<SensorType>>* listener) {
		ASSERT(!this->listeners.isEmpty()) << "No subscription found";

		listener->removeLinkFromList();
		if (this->listeners.isEmpty()) {
			dispatchDisableSensor();
		}
	}

public:

	AndroidSensor() {
	}
	AndroidSensor(ASensorEventQueue* eventqueue, ASensor const* sensor)
			: eventQueue(eventqueue), sensor(sensor) {
	}

	void setAndroidSensor(ASensor const* sensor) {
		this->sensor = sensor;
	}
	ASensor const* getAndroidSensor() {
		return sensor;
	}
};

}  // namespace rhfw

#endif /* ANDROIDPLATFORM_SENSOR_ANDROIDSENSOR_H_ */
