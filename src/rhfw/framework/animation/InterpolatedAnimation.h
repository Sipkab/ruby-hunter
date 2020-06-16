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
 * InterpolatedAnimaiton.h
 *
 *  Created on: 2015 febr. 27
 *      Author: sipka
 */

#ifndef INTERPOLATEDANIMAITON_H_
#define INTERPOLATEDANIMAITON_H_

#include <framework/animation/Animation.h>
#include <gen/configuration.h>
namespace rhfw {

class Interpolator;

class InterpolatedAnimation: public Animation {
private:
	Interpolator* interpolator;
protected:
	virtual void onTimeProgress(float progress) = 0;
public:
	InterpolatedAnimation();
	virtual ~InterpolatedAnimation();

	void onProgress(const core::time_millis& progress) override;

	//bool addXmlChild(const XmlNode& child) override;
};

} // namespace rhfw

#endif /* INTERPOLATEDANIMAITON_H_ */
