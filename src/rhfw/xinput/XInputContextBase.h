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
 * XInputContextBase.h
 *
 *  Created on: 2017. aug. 24.
 *      Author: sipka
 */

#ifndef JNI_XINPUT_XINPUTCONTEXTBASE_H_
#define JNI_XINPUT_XINPUTCONTEXTBASE_H_

#include <win32platform/minwindows.h>

#include <Xinput.h>

namespace rhfw {

class XInputContextBase {
	friend class XInputGamePad;
protected:
	typedef DWORD (WINAPI *PROTO_XInputGetState)( _In_ DWORD dwUserIndex, _Out_ XINPUT_STATE* pState);
	typedef DWORD (WINAPI *PROTO_XInputSetState)( _In_ DWORD dwUserIndex, _In_ XINPUT_VIBRATION* pVibration);
	typedef void (WINAPI *PROTO_XInputEnable)( _In_ BOOL enable);

	PROTO_XInputGetState func_XInputGetState = nullptr;
	PROTO_XInputEnable func_XInputEnable = nullptr;

	virtual void reportGamePadGetStateNotConnected(XInputGamePad* gamepad) = 0;
public:
	virtual ~XInputContextBase() = default;

};

}  // namespace rhfw

#endif /* JNI_XINPUT_XINPUTCONTEXTBASE_H_ */
