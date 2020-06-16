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
 * DirectInputDeviceTypeTester.h
 *
 *  Created on: 2017. aug. 24.
 *      Author: sipka
 */

#ifndef JNI_DIRECTINPUT_DIRECTINPUTDEVICETYPETESTER_H_
#define JNI_DIRECTINPUT_DIRECTINPUTDEVICETYPETESTER_H_

#include <win32platform/minwindows.h>

#include <wbemidl.h>
#include <oleauto.h>

namespace rhfw {

class DirectInputDeviceTypeTester {
protected:
	IWbemLocator* pIWbemLocator = nullptr;
	IEnumWbemClassObject* pEnumDevices = nullptr;
	IWbemServices* pIWbemServices = nullptr;

	BSTR bstrDeviceID = NULL;
public:
	DirectInputDeviceTypeTester();
	~DirectInputDeviceTypeTester();

	BOOL IsXInputDevice(const GUID* pGuidProductFromDirectInput);
};

} // namespace rhfw

#endif /* JNI_DIRECTINPUT_DIRECTINPUTDEVICETYPETESTER_H_ */
