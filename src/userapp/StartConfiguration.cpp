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
 * StartConfiguration.cpp
 *
 *  Created on: 2016. szept. 10.
 *      Author: sipka
 */

#include <framework/io/stream/BufferedInputStream.h>
#include <StartConfiguration.h>
#include <string.h>
#include <stdio.h>

#define writeString(str) write(str, sizeof(str) - 1)
#define peekString(str) peekEquals(str, sizeof(str) - 1)

namespace userapp {
using namespace rhfw;

const RenderMap RenderMap::instance;

void StartConfiguration::load(FileDescriptor& fd) {
	auto in = fd.createInput();
	if (in->open()) {
		auto stream = BufferedInputStream::wrap(*in);
		//int read = stream.read(buffer, sizeof(buffer));
		while (true) {
			if (stream.peekString("renderer=")) {
				stream.skip(sizeof("renderer=") - 1);

				for (unsigned int i = 0; i < (unsigned int) RenderConfig::_count_of_entries; ++i) {
					auto&& id = RenderMap::instance.ids[i];
					auto idlen = strlen(id);
					if (stream.peekEquals(id, idlen)) {
						renderConfig = RenderMap::instance.configs[i];
						stream.skip(idlen);
						LOGI()<< "Loaded render config: " << renderConfig;
						break;
					}
				}
			} else if (stream.peekString("vsync=")) {
				stream.skip(sizeof("vsync=") - 1);
				if (stream.peekString("on\n")) {
					stream.skip(sizeof("on\n") - 1);
					vsyncOptions = VSyncOptions::VSYNC_ON;
				} else if (stream.peekString("off\n")) {
					stream.skip(sizeof("off\n") - 1);
					vsyncOptions = VSyncOptions::VSYNC_OFF;
				} else if (stream.peekString("adaptive\n")) {
					stream.skip(sizeof("adaptive\n") - 1);
					vsyncOptions = VSyncOptions::ADAPTIVE;
				}
			} else if (stream.peekString("msaa=")) {
				stream.skip(sizeof("msaa=") - 1);
				char buf[32];
				unsigned int l = 0;
				while (l < sizeof(buf) - 1 && stream.read(buf + l, 1) == 1 && buf[l] >= '0' && buf[l] <= '9') {
					++l;
				}
				//the last read is '\n'
				buf[l] = 0;
				unsigned int factor;
				int sres = sscanf(buf, "%u", &factor);
				if (sres == 1) {
					this->msaaFactor = factor;
				}
			} else if (stream.peekString("server=")) {
				stream.skip(sizeof("server=") - 1);
				char buf[256];
				unsigned int l = 0;
				while (l < sizeof(buf) - 1 && stream.read(buf + l, 1) == 1 && buf[l] >= 32 && buf[l] <= 126) {
					++l;
				}
				//the last read is '\n'
				unsigned int b1;
				unsigned int b2;
				unsigned int b3;
				unsigned int b4;
				unsigned int port;
				buf[l] = 0;
				int sres = sscanf(buf, "%u.%u.%u.%u:%u", &b1, &b2, &b3, &b4, &port);
				if (sres == 5) {
					this->serverAddress = TCPIPv4Address { IPv4Address { (uint8) b1, (uint8) b2, (uint8) b3, (uint8) b4 }, (uint16) port };
				}
			} else {
				break;
			}
		}
	}
	delete in;
}

void StartConfiguration::save(FileDescriptor& fd) {
	auto out = fd.createOutput();
	if (out->open()) {
		if (renderConfig != RenderConfig::_count_of_entries) {
			out->writeString("renderer=");
			auto&& id = RenderMap::instance.ids[(unsigned int) renderConfig];
			out->write(id, strlen(id));
			//id contains '\n'
		}
		switch (vsyncOptions) {
			case VSyncOptions::VSYNC_ON: {
				out->writeString("vsync=");
				out->writeString("on\n");
				break;
			}
			case VSyncOptions::VSYNC_OFF: {
				out->writeString("vsync=");
				out->writeString("off\n");
				break;
			}
			case VSyncOptions::ADAPTIVE: {
				out->writeString("vsync=");
				out->writeString("adaptive\n");
				break;
			}
			default: {
				break;
			}
		}
		{
			char buf[256];
			int len = snprintf(buf, sizeof(buf), "msaa=%u\n", msaaFactor);
			out->write(buf, len);
		}

		if (serverAddress.getPort() != 0) {
			char buf[256];
			int len = snprintf(buf, sizeof(buf), "server=%u.%u.%u.%u:%u\n", serverAddress.getAddressBytes()[0], serverAddress.getAddressBytes()[1],
					serverAddress.getAddressBytes()[2], serverAddress.getAddressBytes()[3], serverAddress.getPort());
			out->write(buf, len);
		}
	}
	delete out;
}

}  // namespace userapp
