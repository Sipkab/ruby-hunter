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
 * DirectInputGamePadContext.cpp
 *
 *  Created on: 2017. aug. 24.
 *      Author: sipka
 */

#include <directinput/DirectInputGamePadContext.h>

#include <wbemidl.h>
#include <oleauto.h>

#include <string.h>
#include <cwchar>

namespace rhfw {

DirectInputGamePadContext::DirectInputGamePadContext() {
	if (!directInputLibrary.load("dinput8.dll")) {
		THROW();
		return;
	}

	func_DirectInput8Create = (PROTO_DirectInput8Create) GetProcAddress((HMODULE) directInputLibrary, "DirectInput8Create");
	void* dinputptr;
	HRESULT res = func_DirectInput8Create(GetModuleHandle(NULL), DIRECTINPUT_VERSION, IID_IDirectInput8, &dinputptr, NULL);
	ASSERT(SUCCEEDED(res)) << res;
}
DirectInputGamePadContext::~DirectInputGamePadContext() {
}

void DirectInputGamePadContext::addGamePadStateListener(GamePadStateListener* listener) {
	listeners.add(listener);
}
void DirectInputGamePadContext::removeGamePadStateListener(GamePadStateListener* listener) {
	listeners.removeOne(listener);
}

unsigned int DirectInputGamePadContext::getGamePadCount() {
	return gamePadList.size();
}

void DirectInputGamePadContext::reportGamePadGetStateNotConnected(DirectInputGamePad* gamepad) {
	//TODO
}

DirectInputGamePad* DirectInputGamePadContext::getGamePad(unsigned int index) {
	return gamePadList.get(index);
}

} // namespace rhfw
