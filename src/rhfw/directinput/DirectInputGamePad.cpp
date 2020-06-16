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
 * DirectInputGamePad.cpp
 *
 *  Created on: 2017. aug. 24.
 *      Author: sipka
 */

#include <InitGuid.h>

#include <directinput/DirectInputGamePad.h>
#include <directinput/DirectInputContextBase.h>

#include <gen/log.h>
#include <gen/types.h>

namespace rhfw {

//taken from DirectInput sample
//https://code.msdn.microsoft.com/vstudio/DirectInput-Samples-8ac6f5e3/sourcecode?fileId=121930&pathId=1254520005
#ifndef DIDFT_OPTIONAL
#define DIDFT_OPTIONAL      0x80000000
#endif

DirectInputGamePad::DirectInputGamePad(DirectInputContextBase* context, const GUID& guid)
		: GamePad(), context(context), guid(guid) {
}
DirectInputGamePad::~DirectInputGamePad() {
	if (device != nullptr) {
		device->Unacquire();
		device->Release();
	}
}

BOOL DirectInputGamePad::EnumDeviceObjectsCallback(LPCDIDEVICEOBJECTINSTANCE lpddoi, LPVOID pvRef) {
	ButtonQuery& query = *reinterpret_cast<ButtonQuery*>(pvRef);
	unsigned int instance = DIDFT_GETINSTANCE(lpddoi->dwType);
	if (IsEqualGUID(GUID_Button, lpddoi->guidType)) {
		if (instance > query.buttonInstances) {
			query.buttonInstances = instance;
		}
	} else if (IsEqualGUID(GUID_POV, lpddoi->guidType)) {
		query.pov = true;
	} else if (IsEqualGUID(GUID_XAxis, lpddoi->guidType)) {
		query.axisX = true;
	} else if (IsEqualGUID(GUID_YAxis, lpddoi->guidType)) {
		query.axisY = true;
	} else if (IsEqualGUID(GUID_ZAxis, lpddoi->guidType)) {
		query.axisZ = true;
	} else if (IsEqualGUID(GUID_RxAxis, lpddoi->guidType)) {
		query.axisRotationX = true;
	} else if (IsEqualGUID(GUID_RyAxis, lpddoi->guidType)) {
		query.axisRotationY = true;
	} else if (IsEqualGUID(GUID_RzAxis, lpddoi->guidType)) {
		query.axisRotationZ = true;
	}

	LOGTRACE() << lpddoi->tszName << " offset: " << lpddoi->dwOfs << " type: " << (lpddoi->dwType & 0xFF) << " instance: " << instance;
	return DIENUM_CONTINUE;
}
HRESULT DirectInputGamePad::initialize(IDirectInputDevice8* device) {
//TODO
	HRESULT hr;
	hr = device->EnumObjects(EnumDeviceObjectsCallback, &query, DIDFT_ALL);
	ASSERT(SUCCEEDED(hr)) << hr;
	if (FAILED(hr)) {
		return hr;
	}
	unsigned int datacount = 0;
	unsigned int buttoninstancecount = query.buttonInstances + 1;
	DWORD buttoninstancenumber = 0;
	DIOBJECTDATAFORMAT rgodf[InputDataFormat::MAX_HANDLED_OBJECTS] = { };
	if (query.pov) {
		rgodf[datacount++] = {&GUID_POV, FIELD_OFFSET(DirectInputGamePad::InputDataFormat, pov), DIDFT_POV | DIDFT_ANYINSTANCE, 0,};
	}
	if (query.axisX) {
		rgodf[datacount++] = {&GUID_XAxis, FIELD_OFFSET(DirectInputGamePad::InputDataFormat, axisX), DIDFT_AXIS | DIDFT_ANYINSTANCE, 0,};
	}
	if (query.axisY) {
		rgodf[datacount++] = {&GUID_YAxis, FIELD_OFFSET(DirectInputGamePad::InputDataFormat, axisY), DIDFT_AXIS | DIDFT_ANYINSTANCE, 0,};
	}
	if (query.axisZ) {
		rgodf[datacount++] = {&GUID_ZAxis, FIELD_OFFSET(DirectInputGamePad::InputDataFormat, axisZ), DIDFT_AXIS | DIDFT_ANYINSTANCE, 0,};
	}
	if (query.axisRotationX) {
		rgodf[datacount++] = {&GUID_RxAxis, FIELD_OFFSET(DirectInputGamePad::InputDataFormat, axisRotationX), DIDFT_AXIS | DIDFT_ANYINSTANCE, 0,};
	}
	if (query.axisRotationY) {
		rgodf[datacount++] = {&GUID_RyAxis, FIELD_OFFSET(DirectInputGamePad::InputDataFormat, axisRotationY), DIDFT_AXIS | DIDFT_ANYINSTANCE, 0,};
	}
	if (query.axisRotationZ) {
		rgodf[datacount++] = {&GUID_RzAxis, FIELD_OFFSET(DirectInputGamePad::InputDataFormat, axisRotationZ), DIDFT_AXIS | DIDFT_ANYINSTANCE, 0,};
	}
	if (buttoninstancecount >= 4) {
		rgodf[datacount++] = {&GUID_Button, FIELD_OFFSET(DirectInputGamePad::InputDataFormat, bButtonA), (DWORD)DIDFT_BUTTON | DIDFT_MAKEINSTANCE(buttoninstancenumber++), 0,};
		rgodf[datacount++] = {&GUID_Button, FIELD_OFFSET(DirectInputGamePad::InputDataFormat, bButtonB), (DWORD)DIDFT_BUTTON | DIDFT_MAKEINSTANCE(buttoninstancenumber++), 0,};
		rgodf[datacount++] = {&GUID_Button, FIELD_OFFSET(DirectInputGamePad::InputDataFormat, bButtonX), (DWORD)DIDFT_BUTTON | DIDFT_MAKEINSTANCE(buttoninstancenumber++), 0,};
		rgodf[datacount++] = {&GUID_Button, FIELD_OFFSET(DirectInputGamePad::InputDataFormat, bButtonY), (DWORD)DIDFT_BUTTON | DIDFT_MAKEINSTANCE(buttoninstancenumber++), 0,};
		buttoninstancecount -= 4;
		query.buttonsAXYZ = true;
	}
	if (buttoninstancecount >= 2) {
		rgodf[datacount++] = {&GUID_Button, FIELD_OFFSET(DirectInputGamePad::InputDataFormat, bButtonLB), (DWORD)DIDFT_BUTTON | DIDFT_MAKEINSTANCE(buttoninstancenumber++), 0,};
		rgodf[datacount++] = {&GUID_Button, FIELD_OFFSET(DirectInputGamePad::InputDataFormat, bButtonRB), (DWORD)DIDFT_BUTTON | DIDFT_MAKEINSTANCE(buttoninstancenumber++), 0,};
		buttoninstancecount -= 2;
		query.buttonsLBRB = true;
	}
	if (!(query.axisZ && !query.axisRotationZ) && buttoninstancecount >= 2) {
		//trigger buttons are represented on the axisZ only if there is no rotation for Z
		rgodf[datacount++] = {&GUID_Button, FIELD_OFFSET(DirectInputGamePad::InputDataFormat, bButtonLT), (DWORD)DIDFT_BUTTON | DIDFT_MAKEINSTANCE(buttoninstancenumber++), 0,};
		rgodf[datacount++] = {&GUID_Button, FIELD_OFFSET(DirectInputGamePad::InputDataFormat, bButtonRT), (DWORD)DIDFT_BUTTON | DIDFT_MAKEINSTANCE(buttoninstancenumber++), 0,};
		buttoninstancecount -= 2;
		query.buttonsLTRT = true;
	}
	if (buttoninstancecount >= 2) {
		rgodf[datacount++] = {&GUID_Button, FIELD_OFFSET(DirectInputGamePad::InputDataFormat, bButtonBack), (DWORD)DIDFT_BUTTON | DIDFT_MAKEINSTANCE(buttoninstancenumber++), 0,};
		rgodf[datacount++] = {&GUID_Button, FIELD_OFFSET(DirectInputGamePad::InputDataFormat, bButtonStart), (DWORD)DIDFT_BUTTON | DIDFT_MAKEINSTANCE(buttoninstancenumber++), 0,};
		buttoninstancecount -= 2;
		query.buttonsBackStart = true;
	}
	if (buttoninstancecount >= 2) {
		rgodf[datacount++] = {&GUID_Button, FIELD_OFFSET(DirectInputGamePad::InputDataFormat, bButtonLThumb), (DWORD)DIDFT_BUTTON | DIDFT_MAKEINSTANCE(buttoninstancenumber++), 0,};
		rgodf[datacount++] = {&GUID_Button, FIELD_OFFSET(DirectInputGamePad::InputDataFormat, bButtonRThumb), (DWORD)DIDFT_BUTTON | DIDFT_MAKEINSTANCE(buttoninstancenumber++), 0,};
		buttoninstancecount -= 2;
		query.buttonsLThumbRRhumb = true;
	}

	DIDATAFORMAT df = { sizeof(DIDATAFORMAT),       // Size of this structure
			sizeof(DIOBJECTDATAFORMAT),	// Size of object data format
			DIDF_ABSAXIS,	// Absolute axis coordinates
			sizeof(DirectInputGamePad::InputDataFormat),	// Size of device data
			datacount,	// Number of objects
			rgodf,	// And here they are
			};

	hr = device->SetDataFormat(&df);
	//E_NOTIMPL for XInput devices?
	ASSERT(SUCCEEDED(hr) || hr == E_NOTIMPL);
	if (FAILED(hr)) {
		LOGE() << "Failed to set data format for DirectInput device " << hr;
		return hr;
	}

	hr = device->Acquire();
	ASSERT(SUCCEEDED(hr));
	if (FAILED(hr)) {
		return hr;
	}
	this->device = device;
	return S_OK;
}

bool DirectInputGamePad::getState(GamePadState* stateout) {
	InputDataFormat data;
	HRESULT hr;
	hr = device->GetDeviceState(sizeof(data), &data);
	switch (hr) {
		case DI_OK: {
			break;
		}
		case DIERR_UNPLUGGED: {
			context->reportGamePadGetStateNotConnected(this);
			return false;
		}
		case DIERR_INPUTLOST:
		case DIERR_NOTACQUIRED: {
			hr = device->Acquire();
			if (FAILED(hr)) {
				if (hr == DIERR_UNPLUGGED) {
					context->reportGamePadGetStateNotConnected(this);
				}
				return false;
			}
			hr = device->GetDeviceState(sizeof(data), &data);
			break;
		}
		default: {
			THROW() << hr;
			break;
		}
	}
	if (FAILED(hr)) {
		LOGE() << "Failed to get device state of DirectInput device " << hr;
		return false;
	}

//	LOGI()<<
//	"Axis XYZ " << data.axisX << " " << data.axisY << " " << data.axisZ << "\n\t" <<
//	"Rotation Axis XYZ " << data.axisRotationX << " " << data.axisRotationY << " " << data.axisRotationZ << "\n\t" <<
//	"ABXY " << (int)data.bButtonA << " "<< (int)data.bButtonB << " "<< (int)data.bButtonX << " "<< (int)data.bButtonY << "\n\t" <<
//	"LB RB LT RT " << (int)data.bButtonLB << " "<< (int)data.bButtonRB << " "<< (int)data.bButtonLT << " "<< (int)data.bButtonRT << "\n\t" <<
//	"Back Start LThumb RRhumb " << (int)data.bButtonBack << " "<< (int)data.bButtonStart << " "<< (int)data.bButtonLThumb << " "<< (int)data.bButtonRThumb << "\n\t" <<
//	"POV " << data.pov;

	stateout->reset();

	stateout->buttons = GamePadButtons::NO_FLAG;

	if (query.pov) {
		DWORD angle = data.pov;
		if (angle <= 36000) {
			const unsigned int STEP = 4500;
			const unsigned int HALFSTEP = 2250;
			//some buttons are down
			/*   */if (angle >= 8 * STEP - HALFSTEP || angle < 0 * STEP + HALFSTEP) {
				stateout->buttons = GamePadButtons::DPAD_UP;
			} else if (angle >= 1 * STEP - HALFSTEP && angle < 1 * STEP + HALFSTEP) {
				stateout->buttons = GamePadButtons::DPAD_UP | GamePadButtons::DPAD_RIGHT;
			} else if (angle >= 2 * STEP - HALFSTEP && angle < 2 * STEP + HALFSTEP) {
				stateout->buttons = GamePadButtons::DPAD_RIGHT;
			} else if (angle >= 3 * STEP - HALFSTEP && angle < 3 * STEP + HALFSTEP) {
				stateout->buttons = GamePadButtons::DPAD_RIGHT | GamePadButtons::DPAD_DOWN;
			} else if (angle >= 4 * STEP - HALFSTEP && angle < 4 * STEP + HALFSTEP) {
				stateout->buttons = GamePadButtons::DPAD_DOWN;
			} else if (angle >= 5 * STEP - HALFSTEP && angle < 5 * STEP + HALFSTEP) {
				stateout->buttons = GamePadButtons::DPAD_DOWN | GamePadButtons::DPAD_LEFT;
			} else if (angle >= 6 * STEP - HALFSTEP && angle < 6 * STEP + HALFSTEP) {
				stateout->buttons = GamePadButtons::DPAD_LEFT;
			} else if (angle >= 7 * STEP - HALFSTEP && angle < 7 * STEP + HALFSTEP) {
				stateout->buttons = GamePadButtons::DPAD_LEFT | GamePadButtons::DPAD_UP;
			} else {
				THROW() << angle;
			}
		}
	}
	if (query.buttonsAXYZ) {
		if (data.bButtonA != 0) {
			stateout->buttons |= GamePadButtons::A;
		}
		if (data.bButtonB != 0) {
			stateout->buttons |= GamePadButtons::B;
		}
		if (data.bButtonX != 0) {
			stateout->buttons |= GamePadButtons::X;
		}
		if (data.bButtonY != 0) {
			stateout->buttons |= GamePadButtons::Y;
		}
	}
	if (query.buttonsLBRB) {
		if (data.bButtonLB != 0) {
			stateout->buttons |= GamePadButtons::LEFT_SHOULDER;
		}
		if (data.bButtonRB != 0) {
			stateout->buttons |= GamePadButtons::RIGHT_SHOULDER;
		}
	}
	if (query.buttonsLTRT) {
		if (data.bButtonLT != 0) {
			stateout->triggerLeft = getTriggerMax();
		}
		if (data.bButtonRT != 0) {
			stateout->triggerRight = getTriggerMax();
		}
	} else {
		//check Z axis
		LONG half = 0x7FFF;
		if (data.axisZ > half) {
			stateout->triggerLeft = data.axisZ - half;
		} else if (data.axisZ < half) {
			stateout->triggerRight = half - data.axisZ;
		}
	}
	if (query.buttonsBackStart) {
		if (data.bButtonBack != 0) {
			stateout->buttons |= GamePadButtons::BACK;
		}
		if (data.bButtonStart != 0) {
			stateout->buttons |= GamePadButtons::START;
		}
	}
	if (query.buttonsLThumbRRhumb) {
		if (data.bButtonLThumb != 0) {
			stateout->buttons |= GamePadButtons::LEFT_THUMB;
		}
		if (data.bButtonRThumb != 0) {
			stateout->buttons |= GamePadButtons::RIGHT_THUMB;
		}
	}
	if (query.axisX && query.axisY) {
		stateout->thumbLeftX = data.axisX;
		//y axis is mirrored
		stateout->thumbLeftY = 0xFFFF - data.axisY;
	}
	if (query.axisRotationX && query.axisRotationY) {
		stateout->thumbRightX = data.axisRotationX;
		//y axis is mirrored
		stateout->thumbRightY = 0xFFFF - data.axisRotationY;
	} else if (query.axisZ && query.axisRotationZ) {
		stateout->thumbRightX = data.axisZ;
		//y axis is mirrored
		stateout->thumbRightY = 0xFFFF - data.axisRotationZ;
	}

	return true;
}

} // namespace rhfw
