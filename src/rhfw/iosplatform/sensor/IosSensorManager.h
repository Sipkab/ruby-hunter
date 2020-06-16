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
 * IosSensorManager.h
 *
 *  Created on: 2016. dec. 10.
 *      Author: sipka
 */

#ifndef IOSPLATFORM_SENSOR_IOSSENSORMANAGER_H_
#define IOSPLATFORM_SENSOR_IOSSENSORMANAGER_H_

#include <framework/io/sensor/SensorManager.h>

namespace rhfw {

class IosSensorManager: public SensorManagerBase {
private:
public:
	virtual bool load() override {
		return false;
	}
	virtual void free() override {
	}
};

}  // namespace rhfw

#endif /* IOSPLATFORM_SENSOR_IOSSENSORMANAGER_H_ */
