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
 * applgue.h
 *
 *  Created on: 2016. dec. 7.
 *      Author: sipka
 */

#ifndef ANDROIDPLATFORM_APPGLUE_H_
#define ANDROIDPLATFORM_APPGLUE_H_

#include <framework/utils/LinkedNode.h>

#include <jni.h>
#include <gen/fwd/types.h>

namespace rhfw {
namespace androidplatform {

UIRotation queryDeviceRotation(JNIEnv& env);

class AndroidDeviceOrientationChangedListener: private LinkedNode<AndroidDeviceOrientationChangedListener> {
	friend UIRotation subscribeDeviceOrientationChange(AndroidDeviceOrientationChangedListener* listener);
	friend void unsubscribeDeviceOrientationChange(AndroidDeviceOrientationChangedListener* listener);

	virtual AndroidDeviceOrientationChangedListener* get() override {
		return this;
	}
public:
	virtual ~AndroidDeviceOrientationChangedListener() = default;

	virtual void onAndroidDeviceOrientationChanged(UIRotation rotation) = 0;
};

UIRotation subscribeDeviceOrientationChange(AndroidDeviceOrientationChangedListener* listener);
void unsubscribeDeviceOrientationChange(AndroidDeviceOrientationChangedListener* listener);

}  // namespace androidplatform
}  // namespace rhfw

#endif /* ANDROIDPLATFORM_APPGLUE_H_ */
