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
 * xaudio2_header.h
 *
 *  Created on: 2016. szept. 3.
 *      Author: sipka
 */

#ifndef XAUDIO2_XAUDIO2_HEADER_H_
#define XAUDIO2_XAUDIO2_HEADER_H_

#if (_WIN32_WINNT >= 0x0602 /*_WIN32_WINNT_WIN8*/)
#include <xaudio2.h>
#else
#include <xaudio2_sdk_pre2_8\XAudio2.h>
#endif

#endif /* XAUDIO2_XAUDIO2_HEADER_H_ */
