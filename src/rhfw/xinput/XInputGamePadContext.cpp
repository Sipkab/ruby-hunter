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
 * XInputGamePadContext.cpp
 *
 *  Created on: 2017. aug. 15.
 *      Author: sipka
 */

#include <framework/threading/Thread.h>
#include <xinput/XInputGamePadContext.h>
#include <gen/log.h>

#include <Dbt.h>

namespace rhfw {
#define WINDOW_CLASSNAME "XInputGamePadWindow"

XInputGamePadContext::XInputGamePadContext() {
	//TODO
	WNDCLASSEX wc;

	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = 0;
	wc.lpfnWndProc = XInputWindowWndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = GetModuleHandle(NULL);
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

	if (!xInputLibrary.load("XInput1_4.dll") && !xInputLibrary.load("XInput9_1_0.dll") && !xInputLibrary.load("xinput1_3.dll")) {
		THROW();
		return;
	}
	func_XInputGetState = (PROTO_XInputGetState) GetProcAddress((HMODULE) xInputLibrary, "XInputGetState");
	func_XInputEnable = (PROTO_XInputEnable) GetProcAddress((HMODULE) xInputLibrary, "XInputEnable");

	enumerateGamePads();

	if (func_XInputEnable != nullptr) {
		func_XInputEnable(TRUE);
	}
}
XInputGamePadContext::~XInputGamePadContext() {
	for (auto&& gp : xInputGamePads) {
		if (gp != nullptr) {
			gamePadList.removeOne(gp);
			for (auto&& l : listeners) {
				l->onGamePadDetached(gp);
			}
			delete gp;
		}
	}
	if (func_XInputEnable != nullptr) {
		func_XInputEnable(FALSE);
	}
	ASSERT(gamePadList.isEmpty());
	listeners.clearWithoutDelete();

	BOOL res;
	res = DestroyWindow(window);
	ASSERT(res) << GetLastError();

	res = UnregisterClassA((LPCSTR) registeredClass, NULL);
	ASSERT(res) << GetLastError();
}

void XInputGamePadContext::enumerateGamePads() {
	for (DWORD i = 0; i < XUSER_MAX_COUNT; i++) {
		XINPUT_STATE state;

		// Simply get the state of the controller from XInput.
		DWORD dwResult = func_XInputGetState(i, &state);

		if (dwResult == ERROR_SUCCESS) {
			// Controller is connected
			if (xInputGamePads[i] == nullptr) {
				LOGI()<< "XInput device connected " << i;
				xInputGamePads[i] = new XInputGamePad(this, i);
				gamePadList.setSorted(xInputGamePads[i], compareGamePads);
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

LRESULT CALLBACK XInputGamePadContext::XInputWindowWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	switch (message) {
		case WM_CREATE: {
			LPVOID createargs = reinterpret_cast<CREATESTRUCT*>(lParam)->lpCreateParams;
			SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR) createargs);
			return 0;
		}
		case WM_DEVICECHANGE: {
			LOGTRACE()<< "WM_DEVICECHANGE " << wParam;
			if(wParam == DBT_DEVNODES_CHANGED) {
				XInputGamePadContext* context = (XInputGamePadContext*) (LONG_PTR) GetWindowLongPtr(hWnd, GWLP_USERDATA);
				context->enumerateGamePads();
			}
			break;
		}
		default: {
			break;
		}
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}

void XInputGamePadContext::addGamePadStateListener(GamePadStateListener* listener) {
	listeners.add(listener);
}
void XInputGamePadContext::removeGamePadStateListener(GamePadStateListener* listener) {
	listeners.removeOne(listener);
}

int XInputGamePadContext::compareGamePads(XInputGamePad* l, XInputGamePad* r) {
	return (int32) l->getGamePadId() - (int32) r->getGamePadId();
}

unsigned int XInputGamePadContext::getGamePadCount() {
	return gamePadList.size();
}

GamePad* XInputGamePadContext::getGamePad(unsigned int index) {
	return gamePadList.get(index);
}

void XInputGamePadContext::reportGamePadGetStateNotConnected(XInputGamePad* gamepad) {
	ASSERT(gamepad != nullptr);
	gamePadList.removeOne(gamepad);
	xInputGamePads[gamepad->getGamePadId()] = nullptr;
	LOGI()<< "XInput device disconnected " << gamepad->getGamePadId();
	for (auto&& l : listeners) {
		l->onGamePadDetached(gamepad);
	}
	delete gamepad;
}

}    // namespace rhfw

