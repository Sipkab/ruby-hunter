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
 * XInputGamePadContext.h
 *
 *  Created on: 2017. aug. 15.
 *      Author: sipka
 */

#ifndef JNI_XINPUT_GAMEPAD_XINPUTGAMEPADCONTEXT_H_
#define JNI_XINPUT_GAMEPAD_XINPUTGAMEPADCONTEXT_H_

#include <framework/io/gamepad/GamePadContext.h>
#include <framework/utils/ArrayList.h>

#include <xinput/XInputGamePad.h>
#include <xinput/XInputContextBase.h>

#include <win32platform/minwindows.h>
#include <win32platform/LibraryHandle.h>

#include <Xinput.h>

namespace rhfw {
class XInputGamePad;
class XInputGamePadContext: public GamePadContext, public XInputContextBase {
	friend class XInputGamePad;
private:

	static int compareGamePads(XInputGamePad* l, XInputGamePad* r);

	ArrayList<GamePadStateListener> listeners;
	ArrayList<XInputGamePad> gamePadList;

	ATOM registeredClass;
	HWND window = NULL;

	LibraryHandle xInputLibrary;

	XInputGamePad* xInputGamePads[XUSER_MAX_COUNT] { };

	static LRESULT CALLBACK XInputWindowWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	void enumerateGamePads();

protected:
	virtual void reportGamePadGetStateNotConnected(XInputGamePad* gamepad) override;
public:
	XInputGamePadContext();
	~XInputGamePadContext();

	virtual void addGamePadStateListener(GamePadStateListener* listener) override;
	virtual void removeGamePadStateListener(GamePadStateListener* listener) override;

	virtual unsigned int getGamePadCount() override;
	virtual GamePad* getGamePad(unsigned int index) override;
};

} // namespace rhfw

#endif /* JNI_XINPUT_GAMEPAD_XINPUTGAMEPADCONTEXT_H_ */
