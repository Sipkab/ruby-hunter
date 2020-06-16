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
 * stam_opt.h
 *
 *  Created on: 2017. aug. 24.
 *      Author: sipka
 */

#ifndef JNI_TEST_SAPPHIRE_STEAM_OPT_H_
#define JNI_TEST_SAPPHIRE_STEAM_OPT_H_

#include <sapphire/sapphireconstants.h>

#if defined(SAPPHIRE_STEAM_API_AVAILABLE)
#include <steam/steam_api.h>
#else
class ISteamUserStats;
class GameOverlayActivated_t;
template<typename T, typename P>
class SteamManualCallbackPlaceHolder {
public:
	typedef void (T::*func_t)(P*);
	void Register( T *pObj, func_t func ) {
	}
	void Unregister() {
	}
};
#define STEAM_CALLBACK(thisclass, func, ...) void func(){}
#define STEAM_CALLBACK_MANUAL(thisclass, func, callback_type, var)	\
		SteamManualCallbackPlaceHolder<thisclass, callback_type> var; void func(callback_type *pParam){}
#endif /* defined(SAPPHIRE_STEAM_API_AVAILABLE) */

#endif /* JNI_TEST_SAPPHIRE_STEAM_OPT_H_ */
