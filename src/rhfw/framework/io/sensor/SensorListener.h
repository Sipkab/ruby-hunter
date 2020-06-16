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
 * SensorListener.h
 *
 *  Created on: 2016. dec. 3.
 *      Author: sipka
 */

#ifndef FRAMEWORK_IO_SENSOR_SENSORLISTENER_H_
#define FRAMEWORK_IO_SENSOR_SENSORLISTENER_H_

#include <framework/utils/LinkedNode.h>
#include <gen/log.h>

namespace rhfw {

template<typename SensorType>
class SensorListener: private LinkedNode<SensorListener<SensorType>> {
	friend SensorType;

	SensorType* sensor = nullptr;

	virtual SensorListener<SensorType>* get() override {
		return this;
	}
public:
	virtual ~SensorListener() {
		if (sensor != nullptr) {
			sensor->unsubscribe(this);
		}
	}

	void subscribe(SensorType* sensor) {
		ASSERT(this->sensor == nullptr) << "SensorListener already subscribed";
		ASSERT(sensor != nullptr) << "Sensor is nullptr";

		sensor->subscribe(this);
		this->sensor = sensor;
	}
	void unsubscribe() {
		ASSERT(this->sensor != nullptr) << "SensorListener is not subscribed";

		this->sensor->unsubscribe(this);
		this->sensor = nullptr;
	}

	virtual void onSensorChanged(const typename SensorType::Data& value) = 0;
};

}  // namespace rhfw

#endif /* FRAMEWORK_IO_SENSOR_SENSORLISTENER_H_ */
