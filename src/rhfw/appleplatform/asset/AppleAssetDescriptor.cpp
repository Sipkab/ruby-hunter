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
 * AppleAssetDescriptor.cpp
 *
 *  Created on: 2015 m�rc. 25
 *      Author: Bence
 */

#include <framework/io/files/AssetFileDescriptor.h>
#include <framework/io/files/FileInput.h>

#include <gen/log.h>
#include <gen/configuration.h>

namespace rhfw {

AppleAssetDescriptor::AppleAssetDescriptor() {
}
AppleAssetDescriptor::AppleAssetDescriptor(RAssetFile assetId)
		: AssetFileDescriptorBase { assetId } {
}
AppleAssetDescriptor::~AppleAssetDescriptor() {
}

}
