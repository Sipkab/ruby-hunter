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
 * DirectInputGamePadContext.h
 *
 *  Created on: 2017. aug. 24.
 *      Author: sipka
 */

#ifndef JNI_DIRECTINPUT_DIRECTINPUTGAMEPADCONTEXT_H_
#define JNI_DIRECTINPUT_DIRECTINPUTGAMEPADCONTEXT_H_

#include <framework/io/gamepad/GamePadContext.h>
#include <framework/utils/ArrayList.h>

#include <directinput/DirectInputGamePad.h>
#include <directinput/DirectInputContextBase.h>

#include <win32platform/minwindows.h>
#include <win32platform/LibraryHandle.h>

#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>

namespace rhfw {

class DirectInputGamePadContext: public GamePadContext, public DirectInputContextBase {
	friend class DirectInputGamePad;
private:

	typedef HRESULT (WINAPI *PROTO_DirectInput8Create)(HINSTANCE hinst, DWORD dwVersion, REFIID riidltf, LPVOID *ppvOut, LPUNKNOWN punkOuter);

	PROTO_DirectInput8Create func_DirectInput8Create = nullptr;

	ArrayList<GamePadStateListener> listeners;
	ArrayList<DirectInputGamePad> gamePadList;

	LibraryHandle directInputLibrary;
protected:
	virtual void reportGamePadGetStateNotConnected(DirectInputGamePad* gamepad) override;
public:
	DirectInputGamePadContext();
	~DirectInputGamePadContext();

	virtual void addGamePadStateListener(GamePadStateListener* listener) override;
	virtual void removeGamePadStateListener(GamePadStateListener* listener) override;

	virtual unsigned int getGamePadCount() override;
	virtual DirectInputGamePad* getGamePad(unsigned int index) override;
};

}
 // namespace rhfw

#endif /* JNI_DIRECTINPUT_DIRECTINPUTGAMEPADCONTEXT_H_ */
