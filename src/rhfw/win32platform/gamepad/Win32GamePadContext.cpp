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
 * Win32GamePadContext.cpp
 *
 *  Created on: 2017. aug. 24.
 *      Author: sipka
 */

#include <win32platform/gamepad/Win32GamePadContext.h>
#include <xinput/XInputGamePad.h>
#include <directinput/DirectInputGamePad.h>

#include <Dbt.h>
#include <Guiddef.h>

namespace rhfw {
#define WINDOW_CLASSNAME "DeviceGamePadWindow"

// Checks whether the specified device supports XInput
// Technique from FDInputJoystickManager::IsXInputDeviceFast in ZDoom
// https://github.com/glfw/glfw/blob/3.2/src/win32_joystick.c#L174
static bool SupportsXInput(const GUID* guid) {
	UINT count;

	if (GetRawInputDeviceList(NULL, &count, sizeof(RAWINPUTDEVICELIST)) != 0) {
		return false;
	}

	RAWINPUTDEVICELIST* ridl = new RAWINPUTDEVICELIST[count];

	if (GetRawInputDeviceList(ridl, &count, sizeof(RAWINPUTDEVICELIST)) == -1) {
		delete[] ridl;
		return false;
	}

	bool result = false;

	for (UINT i = 0; i < count; i++) {
		if (ridl[i].dwType != RIM_TYPEHID)
			continue;

		RID_DEVICE_INFO rdi;
		rdi.cbSize = sizeof(rdi);
		UINT size = sizeof(rdi);

		if ((INT) GetRawInputDeviceInfoA(ridl[i].hDevice, RIDI_DEVICEINFO, &rdi, &size) == -1) {
			continue;
		}

		if (MAKELONG(rdi.hid.dwVendorId, rdi.hid.dwProductId) != guid->Data1)
			continue;

		char name[256] { 0 };
		size = sizeof(name);

		if ((INT) GetRawInputDeviceInfoA(ridl[i].hDevice, RIDI_DEVICENAME, name, &size) == -1) {
			break;
		}

		name[sizeof(name) - 1] = '\0';
		if (strstr(name, "IG_")) {
			result = true;
			break;
		}
	}

	delete[] ridl;
	return result;
}

int Win32GamePadContext::compareXInputGamePads(XInputGamePad* l, XInputGamePad* r) {
	return (int32) l->getGamePadId() - (int32) r->getGamePadId();
}

BOOL CALLBACK EnumGameControllersCallback(const DIDEVICEINSTANCE* pdidInstance, VOID* pContext) {
	Win32GamePadContext* thiz = reinterpret_cast<Win32GamePadContext*>(pContext);
	thiz->directInputDeviceEnumerated(pdidInstance);
	return DIENUM_CONTINUE;
}

Win32GamePadContext::Win32GamePadContext() {
	HINSTANCE processinstance = GetModuleHandle(NULL);

	WNDCLASSEX wc;
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = 0;
	wc.lpfnWndProc = DeviceWindowWndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = processinstance;
	wc.hCursor = NULL;
	wc.hbrBackground = NULL;
	wc.lpszMenuName = NULL;
	wc.lpszClassName = WINDOW_CLASSNAME;
	wc.hIcon = NULL;
	wc.hIconSm = NULL;

	registeredClass = RegisterClassEx(&wc);
	ASSERT(registeredClass != 0) << GetLastError();

	window = CreateWindowEx(0,	//extended style
			wc.lpszClassName,    // name of the window class
			NULL,   // title of the window
			0,    // window style
			0,    // x-position of the window
			0,    // y-position of the window
			0,    // width of the window
			0,    // height of the window
			NULL,    // no parent window, NULL
			NULL,    // not using menus, NULL
			wc.hInstance,    // application handle
			this);    //
	ASSERT(window != NULL) << GetLastError();

	if (directInputLibrary.load("dinput8.dll")) {
		func_DirectInput8Create = (PROTO_DirectInput8Create) GetProcAddress((HMODULE) directInputLibrary, "DirectInput8Create");
		HRESULT hr = func_DirectInput8Create(processinstance, DIRECTINPUT_VERSION, IID_IDirectInput8,
				reinterpret_cast<void**>(&directInputInstance), NULL);
		ASSERT(SUCCEEDED(hr)) << hr;
	}

	if (xInputLibrary.load("XInput1_4.dll") || xInputLibrary.load("XInput9_1_0.dll") || xInputLibrary.load("xinput1_3.dll")) {
		func_XInputGetState = (PROTO_XInputGetState) GetProcAddress((HMODULE) xInputLibrary, "XInputGetState");
		func_XInputEnable = (PROTO_XInputEnable) GetProcAddress((HMODULE) xInputLibrary, "XInputEnable");
	}

	enumerateGamePads();

	if (func_XInputEnable != nullptr) {
		func_XInputEnable(TRUE);
	}
}
Win32GamePadContext::~Win32GamePadContext() {
	auto&& deleter = [&] (GamePad* gp) {
		if (gp != nullptr) {
			for (auto&& l : listeners) {
				l->onGamePadDetached(gp);
			}
		}
	};
	xInputGamePadList.clearHandle(deleter);
	directInputGamePadList.clearHandle(deleter);
	if (func_XInputEnable != nullptr) {
		func_XInputEnable(FALSE);
	}
	if (directInputInstance != nullptr) {
		directInputInstance->Release();
	}
	listeners.clearWithoutDelete();

	BOOL bres;
	bres = DestroyWindow(window);
	ASSERT(bres) << GetLastError();

	bres = UnregisterClassA((LPCSTR) registeredClass, NULL);
	ASSERT(bres) << GetLastError();
}

void Win32GamePadContext::enumerateGamePads() {
	enumerateXInputGamePads();
	enumareteDirectInputGamePads();
}

void Win32GamePadContext::enumerateXInputGamePads() {
	if (func_XInputGetState != nullptr) {
		//XInput
		for (DWORD i = 0; i < XUSER_MAX_COUNT; i++) {
			XINPUT_STATE state;

			// Simply get the state of the controller from XInput.
			DWORD dwResult = func_XInputGetState(i, &state);

			if (dwResult == ERROR_SUCCESS) {
				// Controller is connected
				if (xInputGamePads[i] == nullptr) {
					LOGI()<< "XInput device connected " << i;
					xInputGamePads[i] = new XInputGamePad(this, i);
					xInputGamePadList.setSorted(xInputGamePads[i], compareXInputGamePads);
					for (auto&& l : listeners) {
						l->onGamePadAttached(xInputGamePads[i]);
					}
				}
			} else {
				// Controller is not connected
				auto* gp = xInputGamePads[i];
				if (gp != nullptr) {
					reportGamePadGetStateNotConnected(gp);
				}
			}
		}
	}
}
void Win32GamePadContext::enumareteDirectInputGamePads() {
	if (directInputInstance != nullptr) {
		//TODO check if directinput gamepads are still connected
		HRESULT hr;
		hr = directInputInstance->EnumDevices(DI8DEVCLASS_GAMECTRL, EnumGameControllersCallback, this, DIEDFL_ATTACHEDONLY);
		ASSERT(SUCCEEDED(hr)) << hr;
	}
}

void Win32GamePadContext::addGamePadStateListener(GamePadStateListener* listener) {
	listeners.add(listener);
}

void Win32GamePadContext::removeGamePadStateListener(GamePadStateListener* listener) {
	listeners.removeOne(listener);
}

unsigned int Win32GamePadContext::getGamePadCount() {
	return xInputGamePadList.size() + directInputGamePadList.size();
}

GamePad* Win32GamePadContext::getGamePad(unsigned int index) {
	if (index < xInputGamePadList.size()) {
		return xInputGamePadList.get(index);
	}
	return directInputGamePadList.get(index - xInputGamePadList.size());
}

void Win32GamePadContext::reportGamePadGetStateNotConnected(XInputGamePad* gamepad) {
	ASSERT(gamepad != nullptr);
	LOGI()<< "XInput device disconnected " << gamepad->getGamePadId();

	xInputGamePadList.removeOne(gamepad);
	xInputGamePads[gamepad->getGamePadId()] = nullptr;
	for (auto&& l : listeners) {
		l->onGamePadDetached(gamepad);
	}
	delete gamepad;
}

void Win32GamePadContext::reportGamePadGetStateNotConnected(DirectInputGamePad* gamepad) {
	ASSERT(gamepad != nullptr);
	LOGI()<< "DirectInput device disconnected";

	directInputGamePadList.removeOne(gamepad);
	for (auto&& l : listeners) {
		l->onGamePadDetached(gamepad);
	}
	delete gamepad;
}

DirectInputGamePad* Win32GamePadContext::findGamePadWithGUID(const GUID& guid) {
	for (auto&& gp : directInputGamePadList) {
		if (IsEqualGUID(gp->getGUID(), guid)) {
			return gp;
		}
	}
	return nullptr;
}

void Win32GamePadContext::directInputDeviceEnumerated(const DIDEVICEINSTANCE* pdidInstance) {
	uint8 type = pdidInstance->dwDevType & 0xFF;
	uint8 subtype = (pdidInstance->dwDevType >> 8) & 0xFF;
	LOGI() << "Device type: " << (int)type << " subtype: " << (int)subtype;
	if (type == DI8DEVTYPE_GAMEPAD && subtype == DI8DEVTYPEGAMEPAD_STANDARD) {
		auto* found = findGamePadWithGUID(pdidInstance->guidInstance);
		if (found != nullptr) {
			//do not try to add again
			return;
		}
		if (SupportsXInput(&pdidInstance->guidProduct)) {
			return;
		}
		//non-xinput device

		if (found == nullptr) {
			LOGI()<< "Non-XInput gamepad added" << pdidInstance->tszInstanceName << " - " << pdidInstance->tszProductName;
			IDirectInputDevice8* dev;
			HRESULT hr = directInputInstance->CreateDevice(pdidInstance->guidInstance, &dev, NULL);
			ASSERT(SUCCEEDED(hr));

			if(SUCCEEDED(hr)) {
				DirectInputGamePad* gp = new DirectInputGamePad(this, pdidInstance->guidInstance);
				hr = gp->initialize(dev);
				if(FAILED(hr)) {
					dev->Release();
					delete gp;
					return;
				}
				for (auto&& l : listeners) {
					l->onGamePadAttached(gp);
				}
				directInputGamePadList.add(gp);
			}
		}
	}
}

LRESULT CALLBACK Win32GamePadContext::DeviceWindowWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	switch (message) {
		case WM_CREATE: {
			LPVOID createargs = reinterpret_cast<CREATESTRUCT*>(lParam)->lpCreateParams;
			SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR) createargs);
			return 0;
		}
		case WM_DEVICECHANGE: {
			LOGTRACE()<<"WM_DEVICECHANGE " << wParam;
			if(wParam == DBT_DEVNODES_CHANGED) {
				//post thread message to work around RPC_E_CANTCALLOUT_ININPUTSYNCCALL error
				PostMessage(hWnd, WM_USER, 0, 0);
			}
			break;
		}
		case WM_USER: {
			LOGI() << "Posted WM_USER";
			Win32GamePadContext* context = (Win32GamePadContext*) (LONG_PTR) GetWindowLongPtr(hWnd, GWLP_USERDATA);
			context->enumerateGamePads();
			break;
		}
		default: {
			break;
		}
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}

}
    // namespace rhfw
