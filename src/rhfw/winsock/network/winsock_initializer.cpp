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
 * winsock_initializer.cpp
 *
 *  Created on: 2016. aug. 27.
 *      Author: sipka
 */

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <gen/log.h>

#pragma comment (lib, "Ws2_32.lib")

class winsock_initializer {
public:
	winsock_initializer() {
		WSADATA wsadata;
		//we expect everyone to use WinSock 2.2, since it was introduced in May 1997
		int res = WSAStartup(MAKEWORD(2, 2), &wsadata);
		ASSERT(res == 0) << "Failed to start up WSA: " << WSAGetLastError();

		LOGI()<< "Successful WSA startup";
		LOGI()<< "WSA description: " << wsadata.szDescription;
		LOGI()<< "WSA system status: " << wsadata.szSystemStatus;
	}
	~winsock_initializer() {
		int res = WSACleanup();
		ASSERT(res == 0 || WSAGetLastError() == WSANOTINITIALISED) << "Failed to clean up WSA: " << WSAGetLastError();
	}
};

static winsock_initializer initer;
