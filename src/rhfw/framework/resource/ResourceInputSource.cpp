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
 * ResourceInputSource.cpp
 *
 *  Created on: 2016. marc. 22.
 *      Author: sipka
 */

#include <framework/resource/ResourceInputSource.h>
#include <framework/resource/ResourceManager.h>
#include <framework/utils/utility.h>

namespace rhfw {

InputSource::Data ResourceInputSource::getData() {
	asset = AssetFileDescriptor { ResourceManager::idToFile(res) };
	unsigned int len;
	const char* data = asset.readFully(&len);
	return {this, data, len};
}

void ResourceInputSource::closeData(InputSource::Data& data) {
	asset = AssetFileDescriptor { };
	delete[] static_cast<const char*>(data);
}

}  // namespace rhfw

