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
 * IosSensor.h
 *
 *  Created on: 2016. dec. 10.
 *      Author: sipka
 */

#ifndef IOSPLATFORM_SENSOR_IOSSENSOR_H_
#define IOSPLATFORM_SENSOR_IOSSENSOR_H_

#include <framework/io/sensor/SensorListener.h>

namespace rhfw {

template<typename SensorType>
class IosSensor {
private:
public:
	void subscribe(LinkedNode<SensorListener<SensorType>>* listener) {
	}

	void unsubscribe(LinkedNode<SensorListener<SensorType>>* listener) {
	}
};

}  // namespace rhfw

#endif /* IOSPLATFORM_SENSOR_IOSSENSOR_H_ */
