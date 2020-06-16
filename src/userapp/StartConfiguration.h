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
 * StartConfiguration.h
 *
 *  Created on: 2016. szept. 10.
 *      Author: sipka
 */

#ifndef TEST_STARTCONFIGURATION_H_
#define TEST_STARTCONFIGURATION_H_

#include <framework/io/files/FileDescriptor.h>
#include <framework/io/network/ipv4/tcp/TCPIPv4Address.h>

#include <gen/types.h>

namespace userapp {
using namespace rhfw;

class RenderMap {
public:
	static const RenderMap instance;
private:
	template<typename T = RenderConfig, RenderConfig Value = T::OpenGl30>
	void setOpenGl(int) {
		ids[(unsigned int) Value] = "OpenGL\n";
		configs[(unsigned int) Value] = Value;
	}
	template<typename T = RenderConfig>
	void setOpenGl(...) {
	}

	template<typename T = RenderConfig, RenderConfig Value = T::DirectX11>
	void setDirectX11(int) {
		ids[(unsigned int) Value] = "DirectX11\n";
		configs[(unsigned int) Value] = Value;
	}
	template<typename T = RenderConfig>
	void setDirectX11(...) {
	}
public:
	const char* ids[(unsigned int) RenderConfig::_count_of_entries];
	RenderConfig configs[(unsigned int) RenderConfig::_count_of_entries];

	RenderMap() {
		setOpenGl(0);
		setDirectX11(0);
	}
};

class StartConfiguration {
public:
	RenderConfig renderConfig = RenderConfig::_count_of_entries;
	VSyncOptions vsyncOptions = VSyncOptions::VSYNC_ON;
	unsigned int msaaFactor = 0;
	TCPIPv4Address serverAddress;

	void load(FileDescriptor& fd);
	void save(FileDescriptor& fd);
};

}  // namespace userapp

#endif /* TEST_STARTCONFIGURATION_H_ */
