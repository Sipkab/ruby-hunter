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
 * DirectInputGamePad.h
 *
 *  Created on: 2017. aug. 24.
 *      Author: sipka
 */

#ifndef JNI_DIRECTINPUT_DIRECTINPUTGAMEPAD_H_
#define JNI_DIRECTINPUT_DIRECTINPUTGAMEPAD_H_

#include <framework/io/gamepad/GamePad.h>

#include <win32platform/minwindows.h>

#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>

namespace rhfw {
class DirectInputGamePadContext;
class DirectInputContextBase;
class DirectInputGamePad: public GamePad {
private:
	class InputDataFormat {
	public:

		static const unsigned int MAX_HANDLED_OBJECTS = 19;

		DWORD pov;

		LONG axisX;
		LONG axisY;
		LONG axisZ;

		LONG axisRotationX;
		LONG axisRotationY;
		LONG axisRotationZ;

		BYTE bButtonA;
		BYTE bButtonB;
		BYTE bButtonX;
		BYTE bButtonY;

		BYTE bButtonLB;
		BYTE bButtonRB;
		BYTE bButtonLT;
		BYTE bButtonRT;

		BYTE bButtonBack;
		BYTE bButtonStart;

		BYTE bButtonLThumb;
		BYTE bButtonRThumb;
	};
	class ButtonQuery {
	public:
		bool pov = false;

		bool axisX = false;
		bool axisY = false;
		bool axisZ = false;

		bool axisRotationX = false;
		bool axisRotationY = false;
		bool axisRotationZ = false;

		unsigned int buttonInstances = 0;

		bool buttonsAXYZ = false;
		bool buttonsLBRB = false;
		bool buttonsLTRT = false;
		bool buttonsBackStart = false;
		bool buttonsLThumbRRhumb = false;
	};
	static BOOL FAR PASCAL EnumDeviceObjectsCallback(LPCDIDEVICEOBJECTINSTANCE lpddoi, LPVOID pvRef);
	ButtonQuery query;

	static_assert(sizeof(InputDataFormat) % sizeof(DWORD) == 0, "invalid size");
	DirectInputContextBase* context;
	GUID guid;

	IDirectInputDevice8* device = nullptr;
public:
	DirectInputGamePad(DirectInputContextBase* context, const GUID& guid);
	~DirectInputGamePad();

	HRESULT initialize(IDirectInputDevice8* device);

	virtual int32 getThumbMin() override {
		return 0;
	}
	virtual int32 getThumbMax() override {
		return 0xFFFF;
	}

	virtual uint32 getTriggerMax() override {
		return 32767; //0x7FFF;
	}

	virtual int32 getLeftThumbDeadzone() override {
		return 7849; //XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE;
	}
	virtual int32 getRightThumbDeadzone() {
		return 8689; //XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE;
	}
	virtual int32 getTriggerDeadzone() override {
		return 3854 * 2;
	}

	virtual bool getState(GamePadState* stateout) override;

	const GUID& getGUID() const {
		return guid;
	}

	IDirectInputDevice8* getDevice() {
		return device;
	}
};

} // namespace rhfw

#endif /* JNI_DIRECTINPUT_DIRECTINPUTGAMEPAD_H_ */
