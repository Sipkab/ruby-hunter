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
 * InterpolatedAnimaiton.cpp
 *
 *  Created on: 2015 febr. 27
 *      Author: sipka
 */

#include <framework/animation/InterpolatedAnimation.h>
#include <framework/animation/interpolator/Interpolator.h>
#include <framework/xml/XmlNode.h>
#include <gen/xmldecl.h>
#include <gen/log.h>
#include <gen/configuration.h>
namespace rhfw {

InterpolatedAnimation::InterpolatedAnimation()
		: interpolator(nullptr) {
}

InterpolatedAnimation::~InterpolatedAnimation() {
}

void InterpolatedAnimation::onProgress(const core::time_millis& progress) {
	ASSERT(interpolator != nullptr) << "interpolator is null";
	const float percent = (float) ((progress - starttime) / duration);
	onTimeProgress(interpolator->interpolate(percent));
}
/*
 bool InterpolatedAnimation::addXmlChild(const XmlNode& child) {
 switch (child.staticType) {
 case RXml::Elem::Interpolator:
 this->interpolator = static_cast<Interpolator*>(child);
 return true;
 default:
 return Animation::addXmlChild(child);
 }
 }
 */
} // namespace rhfw
