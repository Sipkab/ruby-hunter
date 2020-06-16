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
 * Win32GamePadContext.h
 *
 *  Created on: 2017. aug. 24.
 *      Author: sipka
 */

#ifndef JNI_WIN32PLATFORM_GAMEPAD_WIN32GAMEPADCONTEXT_H_
#define JNI_WIN32PLATFORM_GAMEPAD_WIN32GAMEPADCONTEXT_H_

#include <framework/io/gamepad/GamePadContext.h>
#include <framework/utils/ArrayList.h>

#include <xinput/XInputGamePad.h>
#include <xinput/XInputContextBase.h>

#include <directinput/DirectInputContextBase.h>
#include <directinput/DirectInputDeviceTypeTester.h>

#include <win32platform/minwindows.h>
#include <win32platform/LibraryHandle.h>

namespace rhfw {

class Win32GamePadContext: public GamePadContext, public XInputContextBase, public DirectInputContextBase {
	friend class XInputGamePad;
	friend class DirectInputGamePad;
private:
	static LRESULT CALLBACK DeviceWindowWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	static int compareXInputGamePads(XInputGamePad* l, XInputGamePad* r);
	static int compareDirectInputGamePads(DirectInputGamePad* l, DirectInputGamePad* r);

	ArrayList<GamePadStateListener> listeners;
	ArrayList<XInputGamePad> xInputGamePadList;
	ArrayList<DirectInputGamePad> directInputGamePadList;

	ATOM registeredClass;
	HWND window = NULL;

	LibraryHandle xInputLibrary;
	XInputGamePad* xInputGamePads[XUSER_MAX_COUNT] { };

	LibraryHandle directInputLibrary;
//	DirectInputDeviceTypeTester directInputTypeTester;

	IDirectInput8* directInputInstance = nullptr;

	void enumerateGamePads();
	void enumerateXInputGamePads();
	void enumareteDirectInputGamePads();

	DirectInputGamePad* findGamePadWithGUID(const GUID& guid);
protected:
	virtual void reportGamePadGetStateNotConnected(XInputGamePad* gamepad) override;
	virtual void reportGamePadGetStateNotConnected(DirectInputGamePad* gamepad) override;
public:
	Win32GamePadContext();
	~Win32GamePadContext();

	virtual void addGamePadStateListener(GamePadStateListener* listener) override;
	virtual void removeGamePadStateListener(GamePadStateListener* listener) override;

	virtual unsigned int getGamePadCount() override;
	virtual GamePad* getGamePad(unsigned int index) override;

	//internal callback function
	void directInputDeviceEnumerated(const DIDEVICEINSTANCE* pdidInstance);
};

}
// namespace rhfw

#endif /* JNI_WIN32PLATFORM_GAMEPAD_WIN32GAMEPADCONTEXT_H_ */
