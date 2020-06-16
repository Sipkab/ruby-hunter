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
 * XmlWriter.cpp
 *
 *  Created on: 2015 m�rc. 7
 *      Author: sipka
 */

#include <framework/io/files/FileDescriptor.h>
#include <framework/io/files/FileOutput.h>
#include <framework/xml/XmlWriter.h>
#include <gen/log.h>

namespace rhfw {

XmlWriter<1>::XmlWriter(FileDescriptor& desc)
		: desc(desc), out(nullptr) {
}

XmlWriter<1>::~XmlWriter() {
	WARN(out != nullptr) << "FileOutput wasnt closed in xml writer";
}

void XmlWriter<1>::start() {
	ASSERT(out == nullptr) << "Xml writing already started";
	out = desc.createOutput();
	//TODO write header and stuff
}

void XmlWriter<1>::end() {
	ASSERT(out != nullptr) << "Xml writing wasnt started";
	delete out;
	out = nullptr;
}
}
