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
 * LinearInterpolator.h
 *
 *  Created on: 2015 febr. 27
 *      Author: sipka
 */

#ifndef LINEARINTERPOLATOR_H_
#define LINEARINTERPOLATOR_H_

#include <framework/animation/interpolator/Interpolator.h>

#include <gen/configuration.h>
namespace rhfw {

class LinearInterpolator: public Interpolator {
public:
	LinearInterpolator() {
	}
	~LinearInterpolator() {
	}

	float interpolate(float input) override {
		return input;
	}
};

}

#endif /* LINEARINTERPOLATOR_H_ */
